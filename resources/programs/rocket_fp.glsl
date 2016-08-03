#version 130

in vec2 v_texCoord;
in vec4 v_colour;
out vec4 outColour;
uniform sampler2D s_texture;

void main()
{
	outColour = texture2D( s_texture, v_texCoord ) * v_colour;
}