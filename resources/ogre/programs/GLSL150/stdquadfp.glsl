#version 150

in vec2 UV;

uniform sampler2D diffuseMap;

out vec4 fragColour;

void main(void)
{
    fragColour = texture2D(diffuseMap, UV);
}
