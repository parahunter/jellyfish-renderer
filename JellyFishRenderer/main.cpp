// 02561-03-01

#define JELLYFISHES 100

#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel\Angel.h"

#include "Angel\obj_reader.h"
#include "Jellyfish.h"

using namespace std;
using namespace Angel;

const float SPEED = 100.0f;
const float MIN_SCALE = 25.0f;
const float MAX_HEIGHT = 2000.0f;
// cube for generating jellies
const float MAX_SCALE = 50.0f;
const unsigned int STEPS_PER_DIRECTION = 10;
const float MAX_SPAWN_HEIGHT = 1000.f;
const vec3 cubeOrigin(-MAX_SCALE*0.5*STEPS_PER_DIRECTION, -MAX_SPAWN_HEIGHT ,-MAX_SCALE*0.5*STEPS_PER_DIRECTION);

// end of cube generator
const vec4 BACKGROUND = vec4(93.0/255,145.0/255,201.0/255,1.0);
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

//jelly
GLuint shaderProgram, tentacleShaderProgram;
GLuint projectionUniform,
	modelViewUniform,
	timeUniform;
GLuint positionAttribute,
	normalAttribute;
GLuint vertexArrayObject,
	vertexBuffer;


//tentacle
GLuint tentaclePositionAttribute,
	tentacleNormalAttribute,
	tentacleModelviewUniform,
	tentacleProjectionUniform;
GLuint tentacleVertexArrayObject,
	tentacleTimeUniform,
	tentaclePhaseShiftUniform,
	tentacleVertexBuffer;
/*
struct Vertex {
    vec3 position1;
	vec3 color1;
	vec3 position2;
	vec3 color2;
};
*/
vector<unsigned int> indices;
vector<unsigned int> tentacleIndices;

vector<Jellyfish> jellys;

vec2 sphericalCoordinates; // two first components of spherical coordinates (azimuth and elevation)
float dist = 100; // last component of spherical coordinates
vec2 angleOffset;
vec2 mousePos;
double timeCounter = 0.0;


void loadShader();
void display();
vec3 generateRandomJellyfishPosition();

/*
vector<Vertex> interleaveData(vector<float> &position, vector<float> &color, vector<float> &position2, vector<float> &color2){
	vector<Vertex> interleavedVertexData;
	for (int i=0;i<position.size();i=i+3){
		vec3 positionVec(position[i],position[i+1],position[i+2]);
		vec3 colorVec(color[i],color[i+1],color[i+2]);
		vec3 positionVec2(position2[i],position2[i+1],position2[i+2]);
		vec3 colorVec2(color2[i],color2[i+1],color2[i+2]);

		Vertex v;
		
		= {
			positionVec,
			colorVec,
			positionVec2,
			colorVec2
		};
		
		interleavedVertexData.push_back(v);
	}
	return interleavedVertexData;
}
*/

void uploadData(vector<Vertex> &vertexData, vector<Vertex> &tentacleVertexData)
{
	    
	GLuint stride = sizeof(Vertex);

	//jelly
	glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData[0].position, GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(normalAttribute);
		
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(0));
	glVertexAttribPointer(normalAttribute , 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)));

	//tentacls
	glGenVertexArrays(1, &tentacleVertexArrayObject);
    glBindVertexArray(tentacleVertexArrayObject);
    
    glGenBuffers(1, &tentacleVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tentacleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, tentacleVertexData.size() * sizeof(Vertex), tentacleVertexData[0].position, GL_STATIC_DRAW);
    
	glEnableVertexAttribArray(tentaclePositionAttribute);
	glEnableVertexAttribArray(tentacleNormalAttribute);
		
	glVertexAttribPointer(tentaclePositionAttribute, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(0));
	glVertexAttribPointer(tentacleNormalAttribute , 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)));

}

vector<Vertex> initData(char * mesh1, vector<unsigned int> &indices){
	vector<Vertex> rest; 
		indices.clear(); // this avoid loading indices multiple times
		cout << "Loading file "<<mesh1<<endl;
	
		bool res = readOBJFile(mesh1, rest);
		if (!res){
			cout << "Cannot read " << mesh1 << flush<<endl;
			exit(1);
		}

		cout << "*" << rest.size();
	
	//indices are trivially rendered
	for(int i = 0; i < rest.size(); i++) {
			indices.push_back(i);
	}

	//test for having only triangles.
/*	if(indices.size() % 3 == 1) {
			indices.push_back(0);
			indices.push_back(1);
	} else if(indices.size() % 3 == 2) {
			indices.push_back(0);
	}

	*/
	
	//cout << "Interleaving data" << endl;
	//vector<Vertex> interleavedVertexData = interleaveData(position[0], color[0], position[1], color[1]);
	
	return rest;
}

void loadShader()
{
	tentacleShaderProgram = InitShader("tentacles.vert", "tentacles.frag", "tentacleFragColor");

	//tentacles
	tentacleTimeUniform = glGetUniformLocation(tentacleShaderProgram, "time");
	if (tentacleTimeUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'time' uniform."<<endl;
	}
	tentacleProjectionUniform = glGetUniformLocation(tentacleShaderProgram, "projection");
	if (tentacleProjectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'projection' uniform."<<endl;
	}

	tentacleModelviewUniform = glGetUniformLocation(tentacleShaderProgram, "modelView");
	if (tentacleModelviewUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'modelView' uniform."<<endl;
	}

	tentaclePositionAttribute = glGetAttribLocation(tentacleShaderProgram, "position"); 
	if (tentaclePositionAttribute == GL_INVALID_INDEX){
		cerr << "Shader did not contain/use the 'position' attribute." << endl;
	}
	
	tentacleNormalAttribute = glGetAttribLocation(tentacleShaderProgram, "normal"); 
	if (tentacleNormalAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'normal' attribute." << endl;
	}

	tentaclePhaseShiftUniform = glGetUniformLocation(tentacleShaderProgram, "phaseShift"); 
	if (tentaclePhaseShiftUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'normal' attribute." << endl;
	}
	
	shaderProgram = InitShader("head.vert",  "head.frag", "fragColor");

	timeUniform = glGetUniformLocation(shaderProgram, "time");
	if (timeUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'time' uniform."<<endl;
	}

	projectionUniform = glGetUniformLocation(shaderProgram, "projection");
	if (projectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'projection' uniform."<<endl;
	}

	modelViewUniform = glGetUniformLocation(shaderProgram, "modelView");
	if (modelViewUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'modelView' uniform."<<endl;
	}
	positionAttribute = glGetAttribLocation(shaderProgram, "position"); 
	if (positionAttribute == GL_INVALID_INDEX){
		cerr << "Shader did not contain/use the 'position' attribute." << endl;
	}
	
	normalAttribute = glGetAttribLocation(shaderProgram, "normal"); 
	if (normalAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'normal' attribute." << endl;
	}

}

void updateTitle(){
	static int count = 0;
	if (count == 10){
		char buffer[50];
	
		glutSetWindowTitle(buffer);
		count = 0;
	}
	count++;
}

void display() 
{	   
	glClearColor(BACKGROUND.x,BACKGROUND.y,BACKGROUND.z,BACKGROUND.w);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	double currentTime = ((double)clock())/CLOCKS_PER_SEC;
	float deltatime = currentTime - timeCounter;
	timeCounter = currentTime;

	mat4 view = Translate(0,0,-dist) * RotateX(sphericalCoordinates.y) * RotateY(sphericalCoordinates.x);

	mat4 projection = Perspective(70, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.01, 1000);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//amazing stuff will happen here!!!
	
    //glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
		
	for(int i = 0 ; i < jellys.size() ; i++)
	{
		if(jellys.at(i).position.y > MAX_HEIGHT) {
			jellys.at(i).position = generateRandomJellyfishPosition();
		}
		glBindVertexArray(vertexArrayObject);
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projection);
		glUniform1f(timeUniform, (float)timeCounter);


		jellys.at(i).update(deltatime,view);

		glBindVertexArray(tentacleVertexArrayObject);
		glUseProgram(tentacleShaderProgram);
		glUniform1f(tentacleTimeUniform, (float)timeCounter);
		glUniformMatrix4fv(tentacleProjectionUniform, 1, GL_TRUE, projection);
		
		jellys.at(i).updateTentacles(view, tentaclePhaseShiftUniform);

	}

	glFlush();
	glDisable(GL_BLEND);
	
	glutSwapBuffers();

	updateTitle();
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

}

void keyboard(unsigned char key, int x, int y){
	// allow keyboard access if scroll wheel is unavailable
	if (key == '+') {
		mouseWheel(0, 1, 0,0 );
	} else if (key == '-') {
		mouseWheel(0, -1, 0,0 );
	}
}

void printHelp(){
	cout << "Use mouse drag to rotate around head."<< endl;
	cout << "Use mouse wheel or '+'/'-' to change blending (works when implemented)."<< endl;
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

	for(int i = 0; i < JELLYFISHES; i++) {
		vec3 position = generateRandomJellyfishPosition();
		float scaleFactor = 0.1;

		Jellyfish newJelly(position, vec3(0,0,0),vec3(0,SPEED,0), vec3(scaleFactor,scaleFactor,scaleFactor), &indices[0], indices.size(), &tentacleIndices[0], tentacleIndices.size(), modelViewUniform, tentacleModelviewUniform);
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

	loadShader();

	char * jellyMesh =  "jellyfish_triang.obj";
	char * tentacleMesh =   "jellyfish-tentacles_triang.obj";

	uploadData(initData(jellyMesh, indices), initData(tentacleMesh, tentacleIndices));
	
	initJellys();

	glutMainLoop();
}