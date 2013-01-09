#pragma once
#include "Angel\Angel.h"
#include "Shader.h"
#include <vector>
using namespace std;

class Jellyfish
{
public:
	Jellyfish(vec3& position, 
		      vec3& rotation, 
			  vec3& velocity, 
			  vec3& scale, 
			  vec3& color, 
			  vector<unsigned int>& headMesh,
		      vector<unsigned int>& tentacleMesh,
			  Shader& headShader,
			  Shader& tentacleShader);
	~Jellyfish(void);
	void update(GLuint vao, mat4 &view, mat4 & projection,float timeCounter, float deltatime);
	void updateTentacles(GLuint vao, mat4 &view, mat4 & projection, float timeCounter);

	vector<unsigned int> headMesh;
    vector<unsigned int> tentacleMesh;
	Shader headShader;
	Shader tentacleShader;	
	vec3 position;
	vec3 rotation;
	vec3 velocity;
	vec3 scale;
	vec3 baseColor;
	GLfloat f;
};

