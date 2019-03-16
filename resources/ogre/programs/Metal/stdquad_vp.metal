#include <metal_stdlib>
using namespace metal;

struct VS_INPUT
{
 float4 position	[[attribute(VES_POSITION)]];
 float2 uv0		    [[attribute(VES_TEXTURE_COORDINATES0)]];
 float4 color		  [[attribute(VES_DIFFUSE)]];
};

struct PS_INPUT
{
 float2 uv0;

 float4 gl_Position [[position]];
 float4 color;
};

vertex PS_INPUT main_metal
(
 VS_INPUT input [[stage_in]],
 constant float4x4 &worldViewProj [[buffer(PARAMETER_SLOT)]]
)
{
 PS_INPUT outVs;
 outVs.gl_Position = worldViewProj * input.position;
 outVs.uv0 = input.uv0;
 outVs.color = input.color;

 return outVs;
}
