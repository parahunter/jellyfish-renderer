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

using namespace std;
using namespace Angel;

//jelly
GLuint shaderProgram, tentacleShaderProgram, skyShaderProgram;
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

//sky
GLuint skyModelviewUniform,
	skyProjectionUniform,
	skyPositionAttribute;
GLuint skyVertexArrayObject;


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
vector<unsigned int> skyIndices;

vector<Jellyfish> jellys;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

vec2 sphericalCoordinates; // two first components of spherical coordinates (azimuth and elevation)
float dist = 100; // last component of spherical coordinates
vec2 angleOffset;
vec2 mousePos;
double timeCounter = 0.0;

void loadShader();
void display();
vec3 generateRandomJellyfishPosition();

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

void initSkyShader() {
	skyShaderProgram = InitShader("sky.vert",  "sky.frag", "fragColor");
	skyProjectionUniform = glGetUniformLocation(skyShaderProgram, "projection");
	if (skyProjectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'projection' uniform."<<endl;
	}

	skyModelviewUniform = glGetUniformLocation(skyShaderProgram, "modelView");
	if (skyModelviewUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'modelView' uniform."<<endl;
	}
	skyPositionAttribute = glGetAttribLocation(skyShaderProgram, "position"); 
	if (skyPositionAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'position' attribute." << endl;
	}
}

void loadSkyMesh() {
	vector<Vertex> vertices;
	loadMesh("sphere.obj",vertices, skyIndices);
	const int size = vertices.size();
	vec3* vertArray = new vec3[size];
	for(int i = 0; i < vertices.size(); i++) {
		vertArray[i] = vertices.at(i).position;
	}
	GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertArray, GL_STATIC_DRAW);

    glGenVertexArrays(1, &skyVertexArrayObject);
    glBindVertexArray(skyVertexArrayObject);

	glEnableVertexAttribArray(skyPositionAttribute);
    glVertexAttribPointer(skyPositionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (const GLvoid *)0);

}

void display() 
{	   
	glClearColor(BACKGROUND.x,BACKGROUND.y,BACKGROUND.z,BACKGROUND.w);
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
	glUseProgram(skyShaderProgram);
	glUniformMatrix4fv(skyProjectionUniform, 1, GL_TRUE, projection);
	glUniformMatrix4fv(skyModelviewUniform, 1, GL_TRUE, view*Scale(1000));
	glDrawElements(GL_TRIANGLES, skyIndices.size(), GL_UNSIGNED_INT, &skyIndices[0]);

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

	initSkyShader();
	loadSkyMesh();

	loadShader();

	char * jellyMesh =  "jellyfish_triang.obj";
	char * tentacleMesh =   "jellyfish-tentacles_triang.obj";

	vector<Vertex> jellyVertices;
	loadMesh(jellyMesh,jellyVertices,indices);
	
	vector<Vertex> tentacleVertices;
	loadMesh(tentacleMesh,tentacleVertices,tentacleIndices);

	uploadData(jellyVertices,tentacleVertices);	
	initJellys();
	glutMainLoop();
}