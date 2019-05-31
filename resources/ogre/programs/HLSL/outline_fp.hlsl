struct PS_INPUT
{
  float4 pos : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

Texture2D<float> diffuseMap : register(t0);
sampler2D sampler0;

uniform float4 outlineColor;
uniform float2 texelSize;
uniform int borderSize;

float4 fp30(PS_INPUT inPs) : SV_Target
{
  if(tex2Dlod(sampler0, float4(inPs.uv0.xy, 0.0, 0.0)).w == 0.0) {
    for (int i = -2; i <= +2; i++)
    {
      for (int j = -2; j <= +2; j++)
      {
        if (i == 0 && j == 0)
        {
          continue;
        }

        float2 offset = float2(i, j) * texelSize.xy;

        // and if one of the neighboring pixels is white (we are on the border)
        if (tex2Dlod(sampler0, float4(inPs.uv0.x + offset.x, inPs.uv0.y + offset.y, 0.0, 0.0)).w > 0.0)
        {
          return outlineColor;
        }
      }
    }
  }

  discard;

  // force shader to keep this variable, but do not use it for now, as OGRE 1.9 does not pass it properly
  if(borderSize == 0) {
    return float4(1.0, 0.0, 0.0, 0.0);
  }
	return float4(0.0, 0.0, 0.0, 0.0);
}

float4 fp40(PS_INPUT inPs) : SV_Target
{
  if(tex2Dlod(sampler0, float4(inPs.uv0.xy, 0.0, 0.0)).w == 0.0) {
    for (int i = -borderSize; i <= +borderSize; i++)
    {
      for (int j = -borderSize; j <= +borderSize; j++)
      {
        if (i == 0 && j == 0)
        {
          continue;
        }

        float2 offset = float2(i, j) * texelSize.xy;

        // and if one of the neighboring pixels is white (we are on the border)
        if (tex2Dlod(sampler0, float4(inPs.uv0.x + offset.x, inPs.uv0.y + offset.y, 0.0, 0.0)).w > 0.0)
        {
          return outlineColor;
        }
      }
    }
  }

  discard;
	return float4(0.0, 0.0, 0.0, 0.0);
}
