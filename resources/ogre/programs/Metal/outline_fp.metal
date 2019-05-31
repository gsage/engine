#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
  float2 uv0;
};

struct Params
{
  float4 outlineColor;
  float4 texelSize;
  int borderSize;
};

fragment float4 main_metal(
  PS_INPUT inPs [[stage_in]],
  texture2d<float> tex [[texture(0)]],
  sampler samplerState [[sampler(0)]],
  constant Params &p[[buffer(PARAMETER_SLOT)]]
)
{
  if(tex.sample(samplerState, inPs.uv0).a == 0.0) {
    for (int i = -p.borderSize; i <= +p.borderSize; i++)
    {
      for (int j = -p.borderSize; j <= +p.borderSize; j++)
      {
        if (i == 0 && j == 0)
        {
          continue;
        }

        float2 offset = float2(i, j) * p.texelSize.xy;

        // and if one of the neighboring pixels is white (we are on the border)
        if (tex.sample(samplerState, inPs.uv0 + offset).a > 0.0)
        {
          return p.outlineColor;
        }
      }
    }
  }
  discard_fragment();
  return float4(0.0);
}
