#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;

// 复用你的 UBO！不用再传 view/projection 了
layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}