varying vec2 UV;
uniform mat4 worldViewProj;
attribute vec2 uv0;
attribute vec4 vertex;

void main()
{
	gl_Position = worldViewProj * vertex;
	UV = uv0;
}
