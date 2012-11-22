// 02561-03-01
#include <stdio.h>
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel\Angel.h"

#include "Angel\obj_reader.h"

using namespace std;
using namespace Angel;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

GLuint shaderProgram;
GLuint projectionUniform,
	modelViewUniform,
	blendValueUniform;
GLuint positionAttribute,
	normalAttribute;
GLuint vertexArrayObject,
	vertexBuffer;
/*
struct Vertex {
    vec3 position1;
	vec3 color1;
	vec3 position2;
	vec3 color2;
};
*/
vector<unsigned int> indices;

vec2 sphericalCoordinates; // two first components of spherical coordinates (azimuth and elevation)
float dist = 10; // last component of spherical coordinates
vec2 angleOffset;
vec2 mousePos;
float blendValue = 0.0f;

void loadShader();
void display();

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

void uploadData(vector<Vertex> &vertexData){
	glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData[0].position, GL_STATIC_DRAW);
    
	GLuint stride = sizeof(Vertex);
	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(normalAttribute);
		
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(0));
	glVertexAttribPointer(normalAttribute , 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)));
}

vector<Vertex> initData(char * mesh1){
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
	if(indices.size() % 3 == 1) {
			indices.push_back(0);
			indices.push_back(1);
	} else if(indices.size() % 3 == 2) {
			indices.push_back(0);
	}

	for(int i = 0; i <  indices.size(); i++) {
		cout << "Vertex drawn: " << rest[indices[i]].position << endl; 
		if(i % 3 == 0) {
			cout << endl;
		}
	}
	//cout << "Interleaving data" << endl;
	//vector<Vertex> interleavedVertexData = interleaveData(position[0], color[0], position[1], color[1]);
	
	return rest;
}

void loadShader(){
	shaderProgram = InitShader("blending.vert",  "blending.frag", "fragColor");
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
	/*
	blendValueUniform = glGetUniformLocation(shaderProgram, "blendValue");
	if (blendValueUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'blendValue' uniform."<<endl;
	}
	normalAttribute = glGetAttribLocation(shaderProgram, "normal"); 
	if (normalAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'normal' attribute." << endl;
	}
	*/
}

void updateTitle(){
	static int count = 0;
	if (count == 10){
		char buffer[50];
		sprintf(buffer, "VertexBlend: Blend %.2f", blendValue);
		glutSetWindowTitle(buffer);
		count = 0;
	}
	count++;
}

void display() {	
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	const float timeScale = 0.008f;
	
    glUseProgram(shaderProgram);

	mat4 view = Translate(0,0,-dist) * RotateX(sphericalCoordinates.y) * RotateY(sphericalCoordinates.x);
	mat4 projection = Perspective(70, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.01, 1000);
	glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, view);
	glUniform1fv(blendValueUniform, 1, &blendValue);
    
    // vertex shader uniforms
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
    
	glutSwapBuffers();

	updateTitle();
}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void animate() {	
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

void mouseWheel(int button, int dir, int x, int y) {
	if (dir > 0) {
		blendValue += 0.01;
		if (blendValue > 1.0){
			blendValue = 1.0;
		}
	} else {
		blendValue -= 0.01;
		if (blendValue < 0.0){
			blendValue = 0.0;
		}
	}
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
	char * mesh1 = "cube.obj";

	uploadData(initData(mesh1));
	
	glutMainLoop();
}