#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;
out mat3 TBN;

uniform mat4 model;
layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 lightSpaceMatrix;

void main()
{
    TexCoords = aTexCoord;
    FragPos = vec3(model * vec4(aPos, 1.0));
    // 法线矩阵
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    // ==========================================
    // 构建 TBN 矩阵
    // ==========================================
    // 1. 所有的向量都需要变换到世界空间
    // 使用 normalMatrix (逆转置矩阵) 来处理法线变换，防止缩放导致法线歪掉
    mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);

    // Gram-Schmidt 正交化 (可选，但推荐，修正 T 和 N 不垂直的情况)
    T = normalize(T - dot(T, N) * N);

    // 副切线通常可以直接用 Cross(N, T) 算出来，或者用传进来的 aBitangent
    // 这里我们直接用叉乘算 B，这比传 aBitangent 更省带宽，还能保证手性正确
    vec3 B = cross(N, T);

    TBN = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}