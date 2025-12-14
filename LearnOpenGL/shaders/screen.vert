#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    // 直接输出 NDC 坐标，不需要 MVP 矩阵，因为它本来就是铺满屏幕的
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}