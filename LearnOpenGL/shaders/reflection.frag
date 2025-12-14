#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox; // 采样天空盒

void main()
{
    // 1. 计算入射向量 I (从相机指向片元)
    vec3 I = normalize(Position - cameraPos);

    // 2. 计算反射向量 R
    vec3 R = reflect(I, normalize(Normal));

    // 3. 采样天空盒
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}