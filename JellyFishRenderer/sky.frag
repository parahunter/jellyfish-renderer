#version 150

in vec3 objectCoordinates;
out vec4 fragColor;

const vec4 top = vec4(83, 137, 181, 255) / 255.0;// vec4(164.0/255,211.0f/255,1.0,1.0);
const vec4 bottom =  vec4(5, 26, 43, 255) / 255.0;// vec4(83, 137, 181, 255) / 255.0;//

void main(void)
{
	float blend = (objectCoordinates.y + 1)/2;
	
	fragColor = mix(bottom,top, blend);
	//fragColor = blend * blend * top + 2 * blend * (1-blend) * vec4(1.0,1.0,1.0,1.0) + (1-blend) * (1-blend) * bottom;
}