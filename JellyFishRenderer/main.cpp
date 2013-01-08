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
float dist = 100; // last component of spherical coordinates
vec2 angleOffset;
vec2 mousePos;
double timeCounter = 0.0;

void display();
vec3 generateRandomJellyfishPosition();

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

	//amazing stuff will happen here!!!
	
	
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
		
		jellys.at(i).update(headVertexArrayObject,view,projection,timeCounter,deltatime);
		jellys.at(i).updateTentacles(tentacleVertexArrayObject,view,projection,timeCounter);
	}
	
	glFlush();
	glDisable(GL_BLEND);
	
	glutSwapBuffers();
}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
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