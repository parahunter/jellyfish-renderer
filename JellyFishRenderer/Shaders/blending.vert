#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;

in vec3 position;
in vec3 normal;

out vec4 colorV;

float base_transl = 0.2;

void main (void) {
	vec3 normTransformed = (modelView * vec4(normal, 0.0)).xyz;
	float translucency = pow(1 - abs(dot(normalize(normTransformed), vec3(0.0,0.0,1.0))),2) ;
	translucency = (translucency + base_transl)/(1+base_transl);
    colorV = vec4(1.0, 0.0,0.0, translucency);
	gl_Position = projection * modelView * vec4(position, 1.0);
}