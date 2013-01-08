#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;
uniform float time;
uniform vec3 color;

in vec3 position;
in vec3 normal;

out vec4 colorV;

const float base_transl = 0.2;
const float border_transl = 0.8;
const float attentuation = 2.0;
const vec3 viewDirection = vec3(0,0,1);

const float fadeOutDistance = 100.0;
const float fadeoutInterval = 50.0;

void main (void) 
{
	//woobliness
	float maxY = 26;
	float heightMod = (maxY - position.y) * 3 / maxY ;
	float sinT = 0.9 + (1 + sin(5*time + 0.3 * position.y))*0.05 *  heightMod;

	vec4 screenPos = projection * modelView * vec4(position.x * sinT, position.y, position.z * sinT, 1.0);
	gl_Position = screenPos;

	float distMod = (screenPos.z - fadeOutDistance) / fadeoutInterval;

	//translusency
	vec3 normTransformed = (modelView * vec4(normal, 0.0)).xyz;
	
	float translucency = pow(1 - abs(dot(normalize(normTransformed), viewDirection)),attentuation) ;
	  
	float fin_translucency = translucency*border_transl + (1 - translucency)*base_transl;
	fin_translucency *= mix(1.0, 0.0, distMod);

	colorV = vec4(color, fin_translucency);
}