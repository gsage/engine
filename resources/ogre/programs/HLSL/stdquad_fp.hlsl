struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

sampler2D sampler0;

float4 fp30(PS_INPUT inPs) : SV_TARGET
{
	return tex2D(sampler0, inPs.uv0);
}
