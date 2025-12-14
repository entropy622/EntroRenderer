#version 420 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix; // 光照空间的 View * Projection
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}