#version 150
 
uniform sampler2D fbo_texture;	
uniform float offset;
uniform vec2 pixelSize;
in vec2 f_texcoord;
 
out vec4 fragColor;

void main(void) 
{
	vec2 texcoord = f_texcoord;
	texcoord.x += sin(texcoord.y * 4*2*3.14159 + offset) / 100;
	texcoord.y += sin(texcoord.x * 4*2*3.14159 + offset) / 100;
	
	fragColor =  vec4(  texture2D(fbo_texture, texcoord) );
}