#version 150

uniform mat4 projection;
uniform mat4 modelView;

in vec3 position;
in vec3 normal;

out vec3 objectCoordinates;

void main(void)
{
	gl_Position = projection * modelView * vec4(position,1.0);
	objectCoordinates = normalize(position);
}