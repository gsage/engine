in vec2 UV;
uniform sampler2D diffuseMap;
uniform vec4 texelSize;
uniform vec4 outlineColor;
uniform int borderSize;

out vec4 PixelColor;

void main(void)
{
  if(texture2D(diffuseMap, UV).a == 0.0) {
    for (int i = -borderSize; i <= +borderSize; i++)
    {
      for (int j = -borderSize; j <= +borderSize; j++)
      {
        if (i == 0 && j == 0)
        {
          continue;
        }

        vec2 offset = vec2(i, j) * texelSize.xy;

        // and if one of the neighboring pixels is white (we are on the border)
        if (texture2D(diffuseMap, vec2(UV.x + offset.x, UV.y + offset.y)).a > 0.0)
        {
          PixelColor = outlineColor;
          return;
        }
      }
    }
  }
  discard;
}
