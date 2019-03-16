#version 400

uniform sampler2D silhouette;
uniform vec2 texelSize;
uniform vec4 outlineColor;
uniform int borderSize;

in FragData
{
    smooth vec2 coords;
} frag;

out vec4 PixelColor;

void main()
{
    // if the pixel is black (we are on the silhouette)
    if (texture(silhouette, frag.coords).a == 0.0f)
    {
        for (int i = -borderSize; i <= +borderSize; i++)
        {
            for (int j = -borderSize; j <= +borderSize; j++)
            {
                if (i == 0 && j == 0)
                {
                    continue;
                }

                vec2 offset = vec2(i, j) * texelSize;

                // and if one of the neighboring pixels is white (we are on the border)
                if (texture(silhouette, frag.coords + offset).a > 0.0f)
                {
                    PixelColor = outlineColor;
                    return;
                }
            }
        }
    }

    discard;
}
