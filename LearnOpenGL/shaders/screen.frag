#version 420 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;      // 原图
uniform sampler2D bloomBlur;  // 泛光图
uniform float exposure;       // 曝光度 (建议 0.5 ~ 2.0)
uniform float gamma;          // 伽马 (通常 2.2)
uniform float bloomStrength;  // 【新增】泛光强度 (建议 0.02 ~ 0.1)

// ========================================================
// ACES 拟合公式 (Narkowicz版本)
// 这是目前游戏业界最常用的 Tone Mapping 算法之一
// ========================================================
vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

    // 1. 混合 Bloom (增加控制权)
    // 直接叠加会导致画面变亮变白，乘个系数压一压
    hdrColor += bloomColor * bloomStrength;

    // 2. 应用曝光
    hdrColor *= exposure;

    // 3. 色调映射 (Tone Mapping) - 使用 ACES 替代原来的 exp
    vec3 result = ACESFilm(hdrColor);

    // 4. 伽马校正
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(hdrColor.rgb, 1.0);
}