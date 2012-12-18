#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;
uniform float time;
uniform float phaseShift;

in vec3 position;
in vec3 normal;

out vec4 colorV;

float base_transl = 0.2;
float amplitude = 3;

void main (void) 
{
	//translusency
	vec3 normTransformed = (modelView * vec4(normal, 0.0)).xyz;
	
	float translucency = pow(1 - abs(dot(normalize(normTransformed), vec3(0.0,0.0,1.0))),2) ;
	
	translucency = (translucency + base_transl)/(1+base_transl);
    
	colorV = vec4(1.0, 0.1,0.1, translucency);

	//woobliness
	float maxY = 26;
	float heightMod = (maxY - position.y) * 2 / maxY ;
	float sinT = amplitude * (1 + sin((5)*time + 0.2 * position.y + phaseShift)) * 1.3;

//	colorV = vec4((sin(phaseShift) + 1)*0.5, 0, 0, 1.0);
	gl_Position = projection * modelView * vec4(position.x + sinT, position.y, position.z + sinT, 1.0);
	
}