#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
  float2 uv0;
  float4 color;
};

fragment float4 main_metal(
  PS_INPUT inPs [[stage_in]],
  texture2d<float> tex [[texture(0)]],
  sampler samplerState [[sampler(0)]])
{
  float4 col = inPs.color * tex.sample(samplerState, inPs.uv0);
  return col;
}

