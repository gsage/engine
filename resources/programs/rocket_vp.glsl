#version 130

out vec2 v_texCoord;
out vec4 v_colour;
in vec4 vertex;
in vec4 colour;
in vec4 uv0;
uniform mat4 modelViewProj;

void main()
{	
	// Transforming The Vertex
    gl_Position =  modelViewProj * vertex;
	
    // Passing The Texture Coordinate Of Texture Unit 0 To The Fragment Shader
	v_texCoord = vec2( uv0 );
	v_colour = colour;
}