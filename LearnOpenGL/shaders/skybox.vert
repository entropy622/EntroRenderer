#version 420 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos; // 对于立方体，位置向量就是采样方向向量
    vec4 pos = projection * view * vec4(aPos, 1.0);

    // 【核心技巧】让深度永远是 1.0 (最远)
    gl_Position = pos.xyww;
}