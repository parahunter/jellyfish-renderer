#pragma once
#include "Angel\Angel.h"

class Jellyfish
{
public:
	Jellyfish(vec3& position, vec3& rotation, vec3& velocity, vec3& scale, void* mesh, int bufferSize, void* tentacleMesh, int tentacleBufferSize, GLint modelviewUniform, GLint tentacleModelViewUniform);
	~Jellyfish(void);
	void update(mat4 &view);
	void updateTentacles(mat4 &view);

private:
	void* mesh;
	void* tentacleMesh;
	int bufferSize;
	int tentacleBufferSize;
	GLuint modelviewUniform;
	GLuint tentacleModelViewUniform;
	

	vec3 position;
	vec3 rotation;
	vec3 velocity;
	vec3 scale;
	vec3 baseColor;
	vec3 borderColor;

};

