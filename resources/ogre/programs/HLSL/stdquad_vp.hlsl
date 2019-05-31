struct VS_INPUT
{
	float4 vertex	: POSITION;
	float2 uv0		: TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : POSITION;
	float2 uv0 : TEXCOORD0;
};

uniform float4x4 worldViewProj;

PS_INPUT vp30(VS_INPUT input)
{
	PS_INPUT outPs;
	outPs.pos = mul(worldViewProj, input.vertex);
	outPs.uv0 = input.uv0;
	return outPs;
}
