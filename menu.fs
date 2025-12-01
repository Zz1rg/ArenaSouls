#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D background;
uniform float opacity;

void main()
{
    vec4 texColor = texture(background, TexCoords);
    FragColor = vec4(texColor.rgb, texColor.a * opacity);
}