#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;
uniform float blendValue;

in vec3 position;
in vec3 normal;

out vec4 colorV;

void main (void) {
    colorV = vec4(1.0,1.0,1.0,1.0);
	gl_Position = projection * modelView * vec4(position, 1.0);
}