#pragma once
#include "Angel\Angel.h"

class Jellyfish
{
public:
	Jellyfish(vec3& position, 
		      vec3& rotation, 
			  vec3& velocity, 
			  vec3& scale, 
			  vec3& color, 
			  void* mesh, 
			  int bufferSize, 
			  void* tentacleMesh, 
			  int tentacleBufferSize, 
			  GLint modelviewUniform, 
			  GLint tentacleModelViewUniform,
  			  GLint colorUniform, 
			  GLint tentacleColorUniform);
	~Jellyfish(void);
	void update(float deltatime, mat4 &view);
	void updateTentacles(mat4 &view, GLint phaseUniform);

	void* mesh;
	void* tentacleMesh;
	int bufferSize;
	int tentacleBufferSize;
	GLuint modelviewUniform;
	GLuint tentacleModelViewUniform;
	GLuint headColorUniform;
	GLuint tentacleColorUniform;

	
	vec3 position;
	vec3 rotation;
	vec3 velocity;
	vec3 scale;
	vec3 baseColor;
};

