#version 330 core

in vec4 vertexColor;
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 lightColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(
        texture(texture1, TexCoord),
        texture(texture2, vec2(1.0 - TexCoord.x, TexCoord.y)),
        0.8)
        * vec4(lightColor, 1.0f);
}