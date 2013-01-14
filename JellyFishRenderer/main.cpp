// 02561-03-01

#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel\Angel.h"
#include "Angel\obj_reader.h"

#include "Jellyfish.h"
#include "parameters.h";
#include "HSVColorPicker.h"
#include "Shader.h"

using namespace std;
using namespace Angel;

Shader headShader,tentacleShader,skyShader;

//head
GLuint headVertexArrayObject;
//tentacle
GLuint tentacleVertexArrayObject;
//sky
GLuint skyVertexArrayObject;

vector<unsigned int> indices;
vector<unsigned int> tentacleIndices;
vector<unsigned int> skyIndices;

vector<Jellyfish> jellys;
HSVColorPicker colorPicker;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

vec2 sphericalCoordinates; // two first components of spherical coordinates (azimuth and elevation)
vec2 angleOffset;
vec2 mousePos;
double timeCounter = 0.0;

void display();
vec3 generateRandomJellyfishPosition();
//post processing effect variables
GLuint framebufferPP, framebufferPPTexture, rboDepth;
GLuint vboFramebufferPPVertices, ppVao;
//pp effect program stuff
GLuint programPP, attributeVCoordPP, uniformFramebufferPPTexture;
bool usePostProcessing = true;

//Pors processing effect functions
int initPP()
{
	/* Texture */
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &framebufferPPTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferPPTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Framebuffer to link everything together */
	glGenFramebuffers(1, &framebufferPP);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferPP);
	glBindTexture(GL_TEXTURE_2D, framebufferPPTexture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferPPTexture, 0);

	GLenum status;
	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
		return 0;
	}
	
	glGenVertexArrays(1, &ppVao);
    glBindVertexArray(ppVao);
	
    GLfloat framebufferPP_vertices[] = {-1,-1,   1,-1,  -1,1,  1,1};
	glGenBuffers(1, &vboFramebufferPPVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboFramebufferPPVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(framebufferPP_vertices), framebufferPP_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	programPP = InitShader("Shaders/pp.vert", "Shaders/pp.frag", "fragColor");
  
	char* attribute_name = "v_coord";
	attributeVCoordPP = glGetAttribLocation(programPP, attribute_name);
	if (attributeVCoordPP == -1) 
	{
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}
  
	char* uniform_name = "fbo_texture";
	uniformFramebufferPPTexture = glGetUniformLocation(programPP, uniform_name);
	if (uniformFramebufferPPTexture == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
		return 0;
	}
}

void ReshapePP()
{
	// Rescale framebufferPP and RBO as well
	glBindTexture(GL_TEXTURE_2D, framebufferPPTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void closePP()
{
	glDeleteRenderbuffers(1, &rboDepth);
	glDeleteTextures(1, &framebufferPPTexture);
	glDeleteFramebuffers(1, &framebufferPP);
	glDeleteBuffers(1, &vboFramebufferPPVertices);
	glDeleteProgram(programPP);
}

void drawPP()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	glUseProgram(programPP);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebufferPPTexture);
	glUniform1i(uniformFramebufferPPTexture, 0);
	

	glBindVertexArray(ppVao);

	glEnableVertexAttribArray(attributeVCoordPP);
  
	glUniform1f(glGetUniformLocation(programPP,"time"), timeCounter);

	glBindBuffer(GL_ARRAY_BUFFER, vboFramebufferPPVertices);
	glVertexAttribPointer(attributeVCoordPP, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(attributeVCoordPP);
}

GLuint loadData(vector<Vertex> &vertexData, Shader & shader)
{    
	GLuint stride = sizeof(Vertex);
	GLuint vertexArrayObject;

	glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData[0].position, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.positionAttribute);
	glEnableVertexAttribArray(shader.normalAttribute);
	
	glVertexAttribPointer(shader.positionAttribute, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(0));
	glVertexAttribPointer(shader.normalAttribute, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)));

	return vertexArrayObject;
}

void loadMesh(char * meshPath, vector<Vertex> & vertices, vector<unsigned int> &indices){
	indices.clear(); // this avoid loading indices multiple times
	cout << "Loading file "<<meshPath<<endl;
	bool res = readOBJFile(meshPath, vertices, indices);
	if (!res){
		cout << "Cannot read " << meshPath << flush<<endl;
		exit(1);
	}
}

void loadShader(Shader & shader, char * vertex, char* fragment)
{
	shader.shaderProgram = InitShader(vertex, fragment, "fragColor");
	
	shader.positionAttribute = glGetAttribLocation(shader.shaderProgram, "position"); 
	if (shader.positionAttribute == GL_INVALID_INDEX){
		cerr << "Shader "<<vertex<<" did not contain/use the 'position' attribute." << endl;
	}
	
	shader.normalAttribute = glGetAttribLocation(shader.shaderProgram, "normal"); 
	if (shader.normalAttribute == GL_INVALID_INDEX) {
		cerr << "Shader "<<vertex<<" did not contain/use the 'normal' attribute." << endl;
	}

	shader.timeUniform = glGetUniformLocation(shader.shaderProgram, "time");
	if (shader.timeUniform == GL_INVALID_INDEX) {
		cerr << "Shader "<<vertex<<" did not contain/use the 'time' uniform."<<endl;
	}
	
	shader.projectionUniform = glGetUniformLocation(shader.shaderProgram, "projection");
	if (shader.projectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader "<<vertex<<" did not contain/use the 'projection' uniform."<<endl;
	}

	shader.modelViewUniform = glGetUniformLocation(shader.shaderProgram, "modelView");
	if (shader.modelViewUniform == GL_INVALID_INDEX) {
		cerr << "Shader "<<vertex<<" did not contain/use the 'modelView' uniform."<<endl;
	}

	shader.phaseUniform = glGetUniformLocation(shader.shaderProgram, "phaseShift"); 
	if (shader.phaseUniform == GL_INVALID_INDEX) {
		cerr << "Shader "<<vertex<<" did not contain/use the 'phase' uniform." << endl;
	}

	shader.colorUniform = glGetUniformLocation(shader.shaderProgram, "color"); 
	if (shader.colorUniform == GL_INVALID_INDEX) {
		cerr << "Shader "<<vertex<<" did not contain/use the 'color' uniform." << endl;
	}
}

void loadSkyMesh() 
{
	vector<Vertex> vertices;
	loadMesh("Meshes/sphere.obj",vertices, skyIndices);
	skyVertexArrayObject = loadData(vertices,skyShader);
}

void display() 
{	   
	if(usePostProcessing)
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferPP);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	double currentTime = ((double)clock())/CLOCKS_PER_SEC;
	float deltatime = currentTime - timeCounter;
	timeCounter = currentTime;

	mat4 view = Translate(0,0,-20) * RotateX(sphericalCoordinates.y) * RotateY(sphericalCoordinates.x);
	mat4 projection = Perspective(70, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.01, 10000);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//amazing stuff will happen here!!!
	glBindVertexArray(skyVertexArrayObject);
	glUseProgram(skyShader.shaderProgram);
	glUniformMatrix4fv(skyShader.projectionUniform, 1, GL_TRUE, projection);
	glUniformMatrix4fv(skyShader.modelViewUniform, 1, GL_TRUE, view*Scale(1000));
	glDrawElements(GL_TRIANGLES, skyIndices.size(), GL_UNSIGNED_INT, &skyIndices[0]);

	colorPicker.UpdateColor(deltatime);
	vec3 color = colorPicker.PickColor();
	
	for(int i = 0 ; i < jellys.size() ; i++)
	{
		if(colorPicker.state != COLOR_RANDOM)
			jellys.at(i).baseColor = colorPicker.PickColor();

		if(jellys.at(i).position.y > MAX_HEIGHT) 
		{
			jellys.at(i).position = generateRandomJellyfishPosition();

			if(colorPicker.state == COLOR_RANDOM)
				jellys.at(i).baseColor = colorPicker.PickColor();
		}

		jellys.at(i).update(headVertexArrayObject,view,projection,timeCounter,deltatime);
		jellys.at(i).updateTentacles(tentacleVertexArrayObject,view,projection,timeCounter);
	}
	
	glFinish();
	glDisable(GL_BLEND);
	
	if(usePostProcessing)
		drawPP();

	glutSwapBuffers();
}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	ReshapePP();
}

void animate() 
{
	glutPostRedisplay();
}

void visible(int vis) {
	if (vis == GLUT_VISIBLE)
		glutIdleFunc(animate);
	else
		glutIdleFunc(0);
}

void mouse(int button, int state, int x, int y) {
	if(state == GLUT_DOWN) {
		mousePos = vec2(x,y);
		angleOffset = sphericalCoordinates;
    } 
}

void motion(int x, int y) {	
	const float motionSpeed = 32;
	vec2 deltaAng(M_PI * (mousePos.x - x) / WINDOW_WIDTH, 
		M_PI * (mousePos.y - y) / WINDOW_HEIGHT);
	sphericalCoordinates = angleOffset + deltaAng * motionSpeed;
}

void keyboard(unsigned char key, int x, int y){
	// allow keyboard access if scroll wheel is unavailable
	if(key == 'c')
	{
		colorPicker.UpdateState();

		if(colorPicker.state == COLOR_RANDOM)
		{
			for(int i = 0 ; i < jellys.size() ; i++)
			{
				jellys[i].baseColor = colorPicker.PickColor();
			}
		}

	}
	
	if(key == 'p')
		usePostProcessing = !usePostProcessing;
}

void printHelp()
{
	cout << "Use mouse drag to rotate around head."<< endl;
	cout << "The c key changes the colors of the jellyfishes"<< endl;
	cout << "The p key toggles the post processing effect on/off "<< endl;
}

vec3 generateRandomJellyfishPosition()
{
	unsigned int x = rand() % STEPS_PER_DIRECTION; 
	unsigned int z = rand() % STEPS_PER_DIRECTION;
	float height = ((rand() % 1000) * MAX_SPAWN_HEIGHT)/999;
	return cubeOrigin + vec3(x*MAX_SCALE,height, z*MAX_SCALE);
}

void initJellys()
{
	jellys = vector<Jellyfish>();
	colorPicker = HSVColorPicker(0.f);

	for(int i = 0; i < JELLYFISHES; i++) {
		vec3 position = generateRandomJellyfishPosition();
		float scaleFactor = 0.1;
		float speed = MIN_SPEED + (rand() % 1000) * (MAX_SPEED-MIN_SPEED) / 999;
		vec3 color = colorPicker.PickColor();

		Jellyfish newJelly(position,
						   vec3(0,0,0),
						   vec3(0,speed,0), 
						   vec3(scaleFactor,scaleFactor,scaleFactor), 
						   color, 
						   indices,  
						   tentacleIndices, 
						   headShader, 
						   tentacleShader);
		
		jellys.push_back(newJelly);
	}
}

int main(int argc, char* argv[]) {
	printHelp();
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
	glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("Jellyfish Renderer");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutVisibilityFunc(visible);
	glutIdleFunc(animate);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	glewExperimental = GL_TRUE;  // Added because of http://openglbook.com/glgenvertexarrays-access-violationsegfault-with-glew/

	GLint GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK) {
		printf("ERROR: %s\n", glewGetErrorString(GlewInitResult));
	}

	glDisable(GL_DEPTH_TEST);	

	loadShader(headShader,"Shaders/head.vert","Shaders/head.frag");
	loadShader(tentacleShader,"Shaders/tentacles.vert","Shaders/tentacles.frag");
	loadShader(skyShader,"Shaders/sky.vert","Shaders/sky.frag");
	
	loadSkyMesh();
	
	char * jellyMesh =  "Meshes/jellyfish_triang.obj";
	char * tentacleMesh =   "Meshes/jellyfish-tentacles_triang.obj";

	vector<Vertex> jellyVertices;
	loadMesh(jellyMesh,jellyVertices,indices);
	headVertexArrayObject = loadData(jellyVertices,headShader);

	vector<Vertex> tentacleVertices;
	loadMesh(tentacleMesh,tentacleVertices,tentacleIndices);
	tentacleVertexArrayObject = loadData(tentacleVertices, tentacleShader);

	initJellys();
	initPP();

	glutMainLoop();

	closePP();	
}