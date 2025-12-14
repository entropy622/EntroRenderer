#version 330 core
out vec4 FragColor;

// 保持和 C++ 代码一致的结构体定义
struct Material {
    sampler2D texture_diffuse1;  // 模型本身的颜色贴图
    sampler2D texture_specular1; // MMD模型的高光贴图(SPA/SPH)通常比较特殊，这里可能暂时用不上最好的效果，先保留
    float shininess;             // 高光反光度
};

struct PointLight {
    vec3 position;
    // 暂时忽略衰减参数，为了让光照更像太阳光（平行光）
    float constant; float linear; float quadratic;
    vec3 ambient; vec3 diffuse; vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform PointLight pointLight;
uniform Material material;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(pointLight.position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 获取物体本来的纹理颜色
    vec3 objectColor = texture(material.texture_diffuse1, TexCoords).rgb;

    // ========================================================
    // 卡通着色核心逻辑 (Toon Shading Core)
    // ========================================================

    // --- 1. 卡通漫反射 (Toon Diffuse) ---
    // 计算正常的光照强度 N dot L，范围 [-1.0, 1.0]
    float diffuseFactor = dot(norm, lightDir);

    // 【核心一步】将连续的光照值“切”成离散的色阶
    float toonIntensity;
    // 设定一个阈值（比如 0.3）。
    // 如果光照强度大于阈值，就认为是“亮部”(1.0)
    // 如果小于阈值，就认为是“阴影部”(比如 0.4，不要设为0，否则死黑)
    if (diffuseFactor > 0.3) {
        toonIntensity = 1.0;
    } else {
        toonIntensity = 0.4;
    }

    // 你也可以用 smoothstep 来做一点点非常硬的过渡，防止边缘锯齿太严重：
    // float toonIntensity = smoothstep(0.28, 0.32, diffuseFactor) * 0.6 + 0.4;

    // 计算最终漫反射颜色：光照颜色 * 卡通强度 * 物体纹理颜色
    vec3 finalDiffuse = pointLight.diffuse * toonIntensity * objectColor;


    // --- 2. 卡通高光 (Toon Specular) ---
    vec3 reflectDir = reflect(-lightDir, norm);
    // 计算标准 Phong 高光因子
    float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // 【核心一步】高光切变
    // 只有当高光因子非常强时（比如大于 0.9），才显示一个纯白的高光点。
    // 这模拟了动漫里头发或眼睛上那个锐利的小白点。
    float toonSpecIntensity = (specFactor > 0.9) ? 1.0 : 0.0;

    vec3 finalSpecular = pointLight.specular * toonSpecIntensity;
    // MMD 的高光通常不需要乘物体颜色，就是纯白的光

    // --- 3. 环境光 (Ambient) ---
    // 环境光稍微给一点，防止阴影太死
    vec3 finalAmbient = pointLight.ambient * objectColor * 0.5;


    // 合并结果
    vec3 result = finalAmbient + finalDiffuse + finalSpecular;
    FragColor = vec4(result, 1.0);
}