#version 330 core
out vec4 FragColor;

// 我们的 Model 类会自动填充这个结构体里的纹理
struct Material {
    sampler2D texture_diffuse1;  // 对应 Mesh.h 里的 "texture_diffuse" + "1"
    sampler2D texture_specular1; // 对应 Mesh.h 里的 "texture_specular" + "1"
    float shininess;
};

// 光照参数
struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform PointLight pointLight;
uniform Material material;

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