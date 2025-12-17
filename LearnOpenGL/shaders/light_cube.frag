#version 420 core
out vec4 FragColor;

uniform vec3 color;
void main()
{
    FragColor = vec4(1.0); // 永远是白色 (1,1,1,1)
}