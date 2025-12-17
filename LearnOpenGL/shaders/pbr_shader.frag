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
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;


const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
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

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 albedo = texture(material.texture_diffuse1, TexCoords * uvScale).rgb;
    float metallic = texture(metallicMap, TexCoords * uvScale).r;
    float roughness = texture(roughnessMap, TexCoords * uvScale).r;
    float ao = texture(aoMap, TexCoords * uvScale).r;

    vec3 N = norm;
    vec3 V = viewDir;

    vec3 Lo = vec3(0.0);

    for(int i = 0; i < 4; ++i)
    {
        vec3 L = normalize(pointLights[i].position - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = pointLights[i].diffuse * attenuation;

        vec3 F0 = vec3(0.04);
        F0      = mix(F0, albedo , metallic);
        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);

        vec3 nominator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular     = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}