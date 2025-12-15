#version 420 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;

// 纹理偏移量 (对应 3x3 矩阵的 9 个位置)
const float offset = 1.0 / 300.0;  // 步长：越小越精细，越大越夸张


void main(){
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;

    // 2. 色调映射 (Tone Mapping)
    // 方法 A: Reinhard 色调映射 (最简单，均匀压暗)
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // 方法 B: 曝光色调映射 (Exposure Tone Mapping) - 推荐！
    // 允许我们在亮处和暗处之间做调整，类似相机的曝光
    // exposure 默认设为 1.0，越大画面越亮，越小画面越暗
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // 3. 伽马校正 (Gamma Correction)
    // 这一步也是必须的，之前可能已经在 C++ 开启了 GL_FRAMEBUFFER_SRGB，
    // 但既然我们自己在做后处理，最好手动控制 Gamma
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}
//void main()
//{
//    // 1. 定义 9 个采样坐标（左上、上、右上、左、中、右...）
//    vec2 offsets[9] = vec2[](
//    vec2(-offset,  offset), // 左上
//    vec2( 0.0f,    offset), // 正上
//    vec2( offset,  offset), // 右上
//    vec2(-offset,  0.0f),   // 左
//    vec2( 0.0f,    0.0f),   // 中
//    vec2( offset,  0.0f),   // 右
//    vec2(-offset, -offset), // 左下
//    vec2( 0.0f,    -offset),// 正下
//    vec2( offset, -offset)  // 右下
//    );
//
//    // 2. 定义卷积核 (Sharpen Kernel)
//    // 中间是 9，周围是 -1。意思是：极度强化中间像素，减去周围像素。
//    float kernel[9] = float[](
//    1,  1,  1,
//    1, -8,  1,
//    1,  1,  1
//    );
//
//    // 3. 采样并累加
//    vec3 sampleTex[9];
//    for(int i = 0; i < 9; i++)
//    {
//        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
//    }
//
//    vec3 col = vec3(0.0);
//    for(int i = 0; i < 9; i++)
//    col += sampleTex[i] * kernel[i];
//
//    FragColor = vec4(col, 1.0);
//}