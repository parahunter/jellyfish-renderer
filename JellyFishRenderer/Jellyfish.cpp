#include "Jellyfish.h"


Jellyfish::Jellyfish(vec3& position, vec3& rotation, vec3& velocity, vec3& scale, void* mesh, int bufferSize, void* tentacleMesh, int tentacleBufferSize, GLint modelviewUniform, GLint tentacleModelViewUniform)
	: 
position(position), rotation(rotation), velocity(velocity), scale(scale), mesh(mesh), bufferSize(bufferSize), tentacleMesh(tentacleMesh), tentacleBufferSize(tentacleBufferSize), modelviewUniform(modelviewUniform), tentacleModelViewUniform(tentacleModelViewUniform)
{

}


void Jellyfish::update(float deltatime, mat4 &view)
{
	position += velocity*deltatime;

	mat4 modelView = view * RotateX(rotation.x) * RotateY(rotation.y) * RotateZ(rotation.z) * Scale(scale) * Translate(position);
	
	glUniformMatrix4fv(modelviewUniform, 1, GL_TRUE, modelView);
	
	glDrawElements(GL_TRIANGLES, bufferSize, GL_UNSIGNED_INT, mesh);
}

const int TENTACLES_OUTER_CIRCLE = 10;
const int TENTACLES_INNER_CIRCLE = 8;

void Jellyfish::updateTentacles(mat4 &view, GLint phaseShiftUniform)
{
	
	mat4 model = RotateX(rotation.x) * RotateY(rotation.y) * RotateZ(rotation.z) * Scale(scale) * Translate(position);


	for(int i = 0; i < TENTACLES_OUTER_CIRCLE; i++) 
	{

		mat4 tentacleTransformation = RotateY(i * 360/TENTACLES_OUTER_CIRCLE) * Translate(vec3(20.0f, -68.0f,0));
		mat4 modelView = view * model *  tentacleTransformation;

		modelView[0][0] = 0.5*scale.x;
		//modelView[0][1] = 0;	
		modelView[0][2] = 0;

		modelView[1][0] = 0;
		//modelView[1][1] = scale.y;

		modelView[1][2] = 0;
		
		modelView[2][0] = 0;
		//modelView[2][1] = 0;
		modelView[2][2] = scale.z;

		glUniformMatrix4fv(tentacleModelViewUniform, 1, GL_TRUE, modelView);
		GLfloat f = modelView[0][3] + modelView[1][3] +  modelView[2][3];
		glUniform1f(phaseShiftUniform, f);
		glDrawElements(GL_TRIANGLES, tentacleBufferSize, GL_UNSIGNED_INT, tentacleMesh);
	}

	for(int i = 0; i < TENTACLES_INNER_CIRCLE; i++) {
		mat4 tentacleTransformation = RotateY(i * 360/TENTACLES_OUTER_CIRCLE) * Translate(vec3(15.0f, -68.0f,0));
		mat4 modelView = view * model *  tentacleTransformation;

		modelView[0][0] = 0.5*scale.x;
		//modelView[0][1] = 0;	
		modelView[0][2] = 0;

		modelView[1][0] = 0;
		//modelView[1][1] = scale.y;

		modelView[1][2] = 0;
		
		modelView[2][0] = 0;
		//modelView[2][1] = 0;
		modelView[2][2] = scale.z;

		glUniformMatrix4fv(tentacleModelViewUniform, 1, GL_TRUE, modelView);
		GLfloat f = modelView[0][3] + modelView[1][3] +  modelView[2][3];
		glUniform1f(phaseShiftUniform, f);
		glDrawElements(GL_TRIANGLES, tentacleBufferSize, GL_UNSIGNED_INT, tentacleMesh);
	}
}

Jellyfish::~Jellyfish(void)
{

}
