#include "Jellyfish.h"


Jellyfish::Jellyfish(vec3& position, vec3& rotation, vec3& velocity, vec3& scale, void* mesh, int bufferSize, void* tentacleMesh, int tentacleBufferSize, GLint modelviewUniform, GLint tentacleModelViewUniform)
	: 
position(position), rotation(rotation), velocity(velocity), scale(scale), mesh(mesh), bufferSize(bufferSize), tentacleMesh(tentacleMesh), tentacleBufferSize(tentacleBufferSize), modelviewUniform(modelviewUniform), tentacleModelViewUniform(tentacleModelViewUniform)
{

}

void Jellyfish::update(mat4 &view)
{
	mat4 modelView = view * Translate(position) * RotateX(rotation.x) * RotateY(rotation.y) * RotateZ(rotation.z) * Scale(scale);
	
	glUniformMatrix4fv(modelviewUniform, 1, GL_TRUE, modelView);
	
	glDrawElements(GL_TRIANGLES, bufferSize, GL_UNSIGNED_INT, mesh);
}

const int TENTACLES_OUTER_CIRCLE = 10;
const int TENTACLES_INNER_CIRCLE = 8;

void Jellyfish::updateTentacles(mat4 &view)
{


	mat4 model = Translate(position)  * RotateX(rotation.x) * RotateY(rotation.y) * RotateZ(rotation.z) * Scale(scale);


	for(int i = 0; i < TENTACLES_OUTER_CIRCLE; i++) {
		mat4 tentacleTransformation = RotateY(i * 360/TENTACLES_OUTER_CIRCLE) * Translate(vec3(2.0f, -6.8f,0));
		mat4 modelView = view * tentacleTransformation * model;

		modelView[0][0] = scale.x;
		//modelView[0][1] = 0;	
		modelView[0][2] = 0;

		modelView[1][0] = 0;
		//modelView[1][1] = scale.y;

		modelView[1][2] = 0;
		
		modelView[2][0] = 0;
		//modelView[2][1] = 0;
		modelView[2][2] = scale.z;

		glUniformMatrix4fv(tentacleModelViewUniform, 1, GL_TRUE, modelView);
	
		glDrawElements(GL_TRIANGLES, tentacleBufferSize, GL_UNSIGNED_INT, tentacleMesh);
	}

	for(int i = 0; i < TENTACLES_INNER_CIRCLE; i++) {
		mat4 tentacleTransformation = RotateY(i * 360/TENTACLES_INNER_CIRCLE + 10) * Translate(vec3(1.4f, -6.8f,0));
		mat4 modelView = view * tentacleTransformation * model;

		modelView[0][0] = scale.x;
		//modelView[0][1] = 0;	
		modelView[0][2] = 0;

		modelView[1][0] = 0;
		//modelView[1][1] = scale.y;

		modelView[1][2] = 0;
		
		modelView[2][0] = 0;
		//modelView[2][1] = 0;
		modelView[2][2] = scale.z;

		glUniformMatrix4fv(tentacleModelViewUniform, 1, GL_TRUE, modelView);
	
		glDrawElements(GL_TRIANGLES, tentacleBufferSize, GL_UNSIGNED_INT, tentacleMesh);
	}
}

Jellyfish::~Jellyfish(void)
{

}
