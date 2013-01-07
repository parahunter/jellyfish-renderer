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

using namespace std;
using namespace Angel;


struct Shader {
	GLuint shaderProgram;
	GLuint timeUniform;
	GLuint projectionUniform;
	GLuint modelViewUniform;
	GLuint positionAttribute;
	GLuint normalAttribute;
	GLuint phaseUniform;
	GLuint colorUniform;
};


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
float dist = 100; // last component of spherical coordinates
vec2 angleOffset;
vec2 mousePos;
double timeCounter = 0.0;

void display();
vec3 generateRandomJellyfishPosition();

/* Global stuff for SSE*/
GLuint fbo, fbo_texture, rbo_depth;
GLuint vbo_fbo_vertices;
GLuint program_postproc, attribute_v_coord_postproc, uniform_fbo_texture;
// SS EFFECT FUNCTIONS

void LoadSSEffectShaders()
{
  program_postproc = InitShader("SSEffect.vert", "SSEffect.frag", "fragColor");

  char *attribute_name = "v_coord";
  attribute_v_coord_postproc = glGetAttribLocation(program_postproc, attribute_name);
  if (attribute_v_coord_postproc == -1) {
    fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
    
  }
 
  char *uniform_name = "fbo_texture";
  uniform_fbo_texture = glGetUniformLocation(program_postproc, uniform_name);
  if (uniform_fbo_texture == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
  }
}

int InitSSEfect()
{
	/* Texture */
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &fbo_texture);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
 
  /* Depth buffer */
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, WINDOW_WIDTH, WINDOW_HEIGHT);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
 
  /* Framebuffer to link everything together */
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
  GLenum status;
  if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLfloat fbo_vertices[] = {
    -1, -1,
     1, -1,
    -1,  1,
     1,  1,
  };
  glGenBuffers(1, &vbo_fbo_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ReshapeSSEffect()
{
  // Rescale FBO and RBO as well
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
 
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,  WINDOW_WIDTH, WINDOW_HEIGHT);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void displaySSEffect()
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 
  glUseProgram(program_postproc);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glUniform1i(uniform_fbo_texture, /*GL_TEXTURE*/0);
  glEnableVertexAttribArray(attribute_v_coord_postproc);
 
  glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
  glVertexAttribPointer(
    attribute_v_coord_postproc,  // attribute
    2,                  // number of elements per vertex, here (x,y)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    0,                  // no extra data between each position
    0                   // offset of first element
  );
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(attribute_v_coord_postproc);
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
	
	/*
	//indices are trivially rendered
	for(int i = 0; i < rest.size(); i++) {
			indices.push_back(i);
	}

	//test for having only triangles.
	if(indices.size() % 3 == 1) {
			indices.push_back(0);
			indices.push_back(1);
	} else if(indices.size() % 3 == 2) {
			indices.push_back(0);
	}

	*/
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
	loadMesh("sphere.obj",vertices, skyIndices);
	skyVertexArrayObject = loadData(vertices,skyShader);
}

void display() 
{	   
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	double currentTime = ((double)clock())/CLOCKS_PER_SEC;
	float deltatime = currentTime - timeCounter;
	timeCounter = currentTime;

	mat4 view = Translate(0,0,-dist) * RotateX(sphericalCoordinates.y) * RotateY(sphericalCoordinates.x);

	mat4 projection = Perspective(70, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.01, 10000);

	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(skyVertexArrayObject);
	glUseProgram(skyShader.shaderProgram);
	glUniformMatrix4fv(skyShader.projectionUniform, 1, GL_TRUE, projection);
	glUniformMatrix4fv(skyShader.modelViewUniform, 1, GL_TRUE, view*Scale(1000));
	glDrawElements(GL_TRIANGLES, skyIndices.size(), GL_UNSIGNED_INT, &skyIndices[0]);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
	
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
		glBindVertexArray(headVertexArrayObject);
		glUseProgram(headShader.shaderProgram);
		glUniformMatrix4fv(headShader.projectionUniform, 1, GL_TRUE, projection);
		glUniform1f(headShader.timeUniform, (float)timeCounter);
		
		jellys.at(i).update(deltatime,view);

		glBindVertexArray(tentacleVertexArrayObject);
		glUseProgram(tentacleShader.shaderProgram);
		glUniform1f(tentacleShader.timeUniform, (float)timeCounter);
		glUniformMatrix4fv(tentacleShader.projectionUniform, 1, GL_TRUE, projection);
		jellys.at(i).updateTentacles(view, tentacleShader.phaseUniform);
	}
	
	glFlush();
	glDisable(GL_BLEND);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	displaySSEffect();

	glutSwapBuffers();
}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	ReshapeSSEffect();
}

void animate() 
{
	//timeCounter = clock()/CLOCKS_PER_SEC;

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

void mouseWheel(int button, int dir, int x, int y) 
{
	if(dir > 0) {
		dist -= 5;
	} else {
		dist += 5;
	}
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
	
	if (key == '+') {
		mouseWheel(0, 1, 0,0 );
	} else if (key == '-') {
		mouseWheel(0, -1, 0,0 );
	}
}

void printHelp(){
	cout << "Use mouse drag to rotate around head."<< endl;
	//cout << "Use mouse wheel or '+'/'-' to change blending (works when implemented)."<< endl;
}

vec3 generateRandomJellyfishPosition() {
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

		vec3 color = colorPicker.PickColor();

		Jellyfish newJelly(position,
						   vec3(0,0,0),
						   vec3(0,SPEED,0), 
						   vec3(scaleFactor,scaleFactor,scaleFactor), 
						   color, 
						   &indices[0], 
						   indices.size(), 
						   &tentacleIndices[0], 
						   tentacleIndices.size(), 
						   headShader.modelViewUniform, 
						   tentacleShader.modelViewUniform,
						   headShader.colorUniform,
						   tentacleShader.colorUniform);
		
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
	glutMouseWheelFunc(mouseWheel);
	glutVisibilityFunc(visible);
	glutIdleFunc(animate);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	glewExperimental = GL_TRUE;  // Added because of http://openglbook.com/glgenvertexarrays-access-violationsegfault-with-glew/

	GLint GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK) {
		printf("ERROR: %s\n", glewGetErrorString(GlewInitResult));
	}

	glEnable(GL_DEPTH_TEST);

	loadShader(headShader,"head.vert","head.frag");
	loadShader(tentacleShader,"tentacles.vert","tentacles.frag");
	loadShader(skyShader,"sky.vert","sky.frag");

	InitSSEfect();
	LoadSSEffectShaders();

	loadSkyMesh();
	
	char * jellyMesh =  "jellyfish_triang.obj";
	char * tentacleMesh =   "jellyfish-tentacles_triang.obj";

	vector<Vertex> jellyVertices;
	loadMesh(jellyMesh,jellyVertices,indices);
	headVertexArrayObject = loadData(jellyVertices,headShader);

	vector<Vertex> tentacleVertices;
	loadMesh(tentacleMesh,tentacleVertices,tentacleIndices);
	tentacleVertexArrayObject = loadData(tentacleVertices, tentacleShader);

	initJellys();
	glutMainLoop();
}