#include "Jellyfish.h"
#include "parameters.h"
#include "math.h"


const float frequency = 5.0/(2*M_PI);
const float base_speed = 2.0f;
float elapsed = 0.0f;


Jellyfish::Jellyfish(vec3& position,
		   vec3& rotation, 
		   vec3& velocity, 
		   vec3& scale, 
		   vec3& color, 
		   vector<unsigned int>& headMesh,
		   vector<unsigned int>& tentacleMesh,
		   Shader& headShader,
		   Shader& tentacleShader)
	: 
position(position), 
rotation(rotation), 
velocity(velocity), 
scale(scale), 
baseColor(color), 
headMesh(headMesh), 
tentacleMesh(tentacleMesh), 
headShader(headShader), 
tentacleShader(tentacleShader)
{
	f = ((rand()%1000)/1000.0)*2*M_PI;
}



void Jellyfish::update(GLuint vao, mat4 &view, mat4 & projection,float timeCounter, float deltatime)
{

	position += (velocity * ((1 + sin(elapsed*2*M_PI*frequency + f))/2)) * deltatime;
	elapsed  += deltatime;
	
	mat4 modelView = view * RotateX(rotation.x) * RotateY(rotation.y) * RotateZ(rotation.z) * Scale(scale) * Translate(position);
	
	glBindVertexArray(vao);
	glUseProgram(headShader.shaderProgram);
	glUniformMatrix4fv(headShader.projectionUniform, 1, GL_TRUE, projection);
	glUniform1f(headShader.timeUniform, (float)timeCounter);
	glUniform1f(headShader.phaseUniform, (float)f);
	glUniformMatrix4fv(headShader.modelViewUniform, 1, GL_TRUE, modelView);
	glUniform3fv(headShader.colorUniform, 1, baseColor);


	glDrawElements(GL_TRIANGLES, headMesh.size(), GL_UNSIGNED_INT, &headMesh[0]);
}

void Jellyfish::updateTentacles(GLuint vao, mat4 &view, mat4 & projection, float timeCounter)
{
	glBindVertexArray(vao);
	glUseProgram(tentacleShader.shaderProgram);
	glUniform1f(tentacleShader.timeUniform, (float)timeCounter);
	glUniformMatrix4fv(tentacleShader.projectionUniform, 1, GL_TRUE, projection);
	glUniform1f(tentacleShader.phaseUniform, f);
	glUniform3fv(tentacleShader.colorUniform, 1, baseColor);

	mat4 model = RotateX(rotation.x) * RotateY(rotation.y) * RotateZ(rotation.z) * Scale(scale) * Translate(position);

	for(int i = 0; i < TENTACLES_OUTER_CIRCLE; i++) 
	{
		mat4 tentacleTransformation = RotateY(i * 360/TENTACLES_OUTER_CIRCLE) * Translate(vec3(20.0f, -68.0f,0));
		mat4 modelView = view * model *  tentacleTransformation;

		modelView[0][0] = 0.5*scale.x;
		modelView[0][2] = 0;
		modelView[1][0] = 0;
		modelView[1][2] = 0;
		modelView[2][0] = 0;
		modelView[2][2] = scale.z;

		glUniformMatrix4fv(tentacleShader.modelViewUniform, 1, GL_TRUE, modelView);	
		glDrawElements(GL_TRIANGLES, tentacleMesh.size(), GL_UNSIGNED_INT, &tentacleMesh[0]);
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

		glUniformMatrix4fv(tentacleShader.modelViewUniform , 1, GL_TRUE, modelView);
		glDrawElements(GL_TRIANGLES, tentacleMesh.size(), GL_UNSIGNED_INT, &tentacleMesh[0]);
	}
}

Jellyfish::~Jellyfish(void)
{

}
