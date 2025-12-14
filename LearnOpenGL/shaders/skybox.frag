#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

// 采样器类型是 samplerCube，不是 sampler2D
uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}