#version 420 core
out vec4 FragColor;

// 我们的 Model 类会自动填充这个结构体里的纹理
struct Material {
    sampler2D texture_diffuse1;  // 对应 Mesh.h 里的 "texture_diffuse" + "1"
    sampler2D texture_specular1; // 对应 Mesh.h 里的 "texture_specular" + "1"
    float shininess;
};

// 光照参数
// 定义光照结构体
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
    PointLight pointLight;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform Material material;
uniform vec2 uvScale;

layout(binding = 10) uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
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
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

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
    // 简单的点光源计算
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLight.position - FragPos);

    // 1. 漫反射
    float diff = max(dot(norm, lightDir), 0.0);

    // 2. 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // 3. 衰减
    float distance = length(pointLight.position - FragPos);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

    // 4. 合并结果
    // 注意：这里读取的是 material.texture_diffuse1
    vec3 ambient  = pointLight.ambient  * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse  = pointLight.diffuse  * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    // 注意：这里读取的是 material.texture_specular1
    vec3 specular = pointLight.specular * spec * vec3(texture(material.texture_specular1, TexCoords));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}