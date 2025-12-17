#version 420 core
layout (location = 0) out vec4 FragColor;    // 输出到 colorBuffers[0]
layout (location = 1) out vec4 BrightColor;  // 输出到 colorBuffers[1]

// 保持和 C++ 代码一致的结构体定义
struct Material {
    sampler2D texture_diffuse1;  // 模型本身的颜色贴图
    sampler2D texture_specular1; // MMD模型的高光贴图(SPA/SPH)通常比较特殊，这里可能暂时用不上最好的效果，先保留
    sampler2D texture_normal1;
    float shininess;             // 高光反光度
};

struct PointLight {
    vec3 position;
    float padding1; // std140 对齐占位 (vec3 后面通常需要补齐到 vec4)

    vec3 ambient;
    float padding2;

    vec3 diffuse;
    float padding3;

    vec3 specular;
    float padding4;

    float constant;
    float linear;
    float quadratic;
    float padding5;
};

// 【绑定点 1】
layout (std140, binding = 1) uniform LightBlock {
    PointLight pointLights[4];
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in mat3 TBN;

uniform vec3 viewPos;
uniform Material material;
uniform vec2 uvScale;
uniform bool useNormalMap;
layout(binding = 10) uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{
    vec3 lightDir = normalize(pointLights[0].position - FragPos);
    // 1. 执行透视除法 (虽然正交投影下 w 是 1，但这步是标准流程)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // 2. 变换到 [0,1] 的范围 (因为深度图是 0~1，而坐标是 -1~1)
    projCoords = projCoords * 0.5 + 0.5;

    // 3. 如果超过了视锥体远端，就不算阴影
    if(projCoords.z > 1.0)
    return 0.0;

    // 4. 获取当前像素的深度 (当前深度)
    float currentDepth = projCoords.z;

    // 5. 【阴影偏移 (Shadow Bias)】解决 "阴影痤疮" (Shadow Acne)
    // 根据表面法线和光线的夹角动态调整偏移量
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // 6. PCF (百分比渐进过滤) - 让阴影边缘柔和一点
    // 采样周围 3x3 的像素并取平均值
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // 如果 当前深度 - bias > 记录深度，说明在阴影里
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

void main()
{
    vec3 norm;
    if (useNormalMap) {
        // 1. 采样法线贴图 (注意应用 uvScale)
        // 使用 material.texture_normal1
        vec3 normalMapValue = texture(material.texture_normal1, TexCoords * uvScale).rgb;
        // 2. 从 [0,1] 映射到 [-1,1]
        normalMapValue = normalMapValue * 2.0 - 1.0;
        // 3. 应用 TBN 矩阵转换到世界空间
        norm = normalize(TBN * normalMapValue);
    } else {
        // 如果没有法线贴图，使用几何法线
        norm = normalize(Normal);
    }

    vec3 lightDir = normalize(pointLights[0].position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 获取物体本来的纹理颜色
    vec3 objectColor = texture(material.texture_diffuse1, TexCoords * uvScale).rgb;
    // 透明度测试
    // 如果这个像素太透明了（Alpha < 0.1），直接扔掉，不要写入颜色缓冲，也不要写入深度缓冲
    if(texture(material.texture_diffuse1, TexCoords).a < 0.1)
        discard;
    // ========================================================
    // 卡通着色核心逻辑 (Toon Shading Core)
    // ========================================================

    // --- 1. 卡通漫反射 (Toon Diffuse) ---
    // 计算正常的光照强度 N dot L，范围 [-1.0, 1.0]
    float diffuseFactor = dot(norm, lightDir);

    // 【核心一步】将连续的光照值“切”成离散的色阶
    float toonIntensity;

    float shadow = ShadowCalculation(FragPosLightSpace, norm);
    if (diffuseFactor < 0.3 || shadow > 0.5) {
        toonIntensity = 0.4;
    } else {
        toonIntensity = 1.0;
    }

    // 计算最终漫反射颜色：光照颜色 * 卡通强度 * 物体纹理颜色
    vec3 finalDiffuse = pointLights[0].diffuse * toonIntensity * objectColor;


    // --- 2. 卡通高光 (Toon Specular) ---
    vec3 reflectDir = reflect(-lightDir, norm);
    // 计算标准 Phong 高光因子
    float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // 【核心一步】高光切变
    // 只有当高光因子非常强时（比如大于 0.9），才显示一个纯白的高光点。
    // 这模拟了动漫里头发或眼睛上那个锐利的小白点。
    float toonSpecIntensity = (specFactor > 0.9) ? 1.0 : 0.0;

    vec3 finalSpecular = pointLights[0].specular * toonSpecIntensity;
    // MMD 的高光通常不需要乘物体颜色，就是纯白的光

    // --- 3. 环境光 (Ambient) ---
    // 环境光稍微给一点，防止阴影太死
    vec3 finalAmbient = pointLights[0].ambient * objectColor * 0.5;


    // 合并结果
    vec3 result = finalAmbient + finalDiffuse + finalSpecular;

    FragColor = vec4(result, 1.0);
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    // 阈值设为 1.0 (超过 1.0 的才发光)
    if(brightness > 1.0)
    BrightColor = vec4(result, 1.0);
    else
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0); // 暗的地方存黑色
}