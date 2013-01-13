#version 150
 
uniform sampler2D fbo_texture;	
uniform float time;
uniform vec2 pixelSize;
in vec2 f_texcoord;
 
out vec4 fragColor;

const float amount = 0.007;

void main(void) 
{
	vec2 texcoord = f_texcoord;
	texcoord.x += amount * sin(texcoord.y * 4*2*3.14159 + time);
	texcoord.y += amount * sin(texcoord.x * 4*2*3.14159 + time);
	
	fragColor =  vec4(  texture2D(fbo_texture, texcoord) );
}