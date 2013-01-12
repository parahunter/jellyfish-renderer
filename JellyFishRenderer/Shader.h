#ifndef SHADER_H
#define SHADER_H
 
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

#endif