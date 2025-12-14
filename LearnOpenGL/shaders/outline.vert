#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float outlineWidth; // 此时这个值代表“屏幕上的相对粗细”，不再是单纯的世界单位

void main()
{
    // 1. 计算世界空间坐标
    vec4 worldPos = model * vec4(aPos, 1.0);

    // 2. 计算法线矩阵 (处理缩放和旋转)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 worldNormal = normalize(normalMatrix * aNormal);

    // 3. 【核心算法】计算顶点到摄像机的距离
    // 在 View 空间中，摄像机位于原点 (0,0,0)
    // view * worldPos 把坐标转到了摄像机面前，直接取 length 就是距离
    vec4 viewPos = view * worldPos;
    float dist = length(viewPos.xyz);

    // 4. 根据距离调整描边宽度
    // 距离越远(dist变大)，我们让宽度也变大，这样透视缩小后看起来就是等宽的了
    // 0.001 是一个缩放因子，方便你在 C++ 里填稍微大一点的整数
    float dynamicWidth = outlineWidth * dist * 0.01;

    // 5. 应用外扩
    worldPos.xyz += worldNormal * dynamicWidth;

    gl_Position = projection * view * worldPos;
}