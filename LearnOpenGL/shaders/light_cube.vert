#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;


out vec4 vertexColor; // 为片段着色器指定一个颜色输出
out vec2 TexCoord;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;



void main()
{
    gl_Position =  projection * view * model * vec4(aPos, 1.0); // 注意我们如何把一个vec3作为vec4的构造器的参数
    TexCoord = aTexCoord;
}