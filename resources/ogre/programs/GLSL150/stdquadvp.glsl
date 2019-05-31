#version 150

in vec2 UV;
uniform mat4 worldViewProj;
out vec2 uv0;

void main()
{
	gl_Position = worldViewProj  * gl_Vertex;
	UV = uv0;
}
