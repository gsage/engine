#version 330

in vec2 UV;
uniform sampler2D diffuseMap;
layout( location = 0 ) out vec4 FragColor;

void main(void)
{
    FragColor = texture2D(diffuseMap, UV);
}
