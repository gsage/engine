varying vec2 UV;
uniform sampler2D diffuseMap;

void main(void)
{
    gl_FragColor = texture2D(diffuseMap, UV);
}
