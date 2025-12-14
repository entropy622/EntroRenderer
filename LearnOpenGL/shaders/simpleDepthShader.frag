#version 420 core
// 片段着色器什么都不用干，深度写入是自动的
void main()
{
    // 只采样红色通道 (深度值存这里)
//    float depthValue = texture(screenTexture, TexCoords).r;
//
//    // 【可视化技巧】
//    // 深度值是非线性的，通常在 0.9 到 1.0 之间，肉眼看着全是白的。
//    // 为了看清，我们可以把它拉伸一下，或者只是简单输出。
//    // 如果是透视投影，需要线性化；但因为阴影贴图是正交投影，所以直接输出就行！
//
//    FragColor = vec4(vec3(depthValue), 1.0);
}