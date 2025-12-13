#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;  // 镜面光颜色 (高光的颜色)
    sampler2D emission;
    float shininess; // 反光度 (高光点的聚焦程度)
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform Material material; // 【新】材质结构体
uniform Light light;       // 【新】光源结构体
uniform sampler2D texture1;
uniform sampler2D texture2;



void main()
{
    // 纹理映射
    vec3 baseColor = texture(material.diffuse, TexCoord).rgb;

    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 1. 环境光
    vec3 ambient = light.ambient * baseColor;

    // 2. 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * baseColor);

    // 3. 镜面光
    vec3 specular = vec3(0.0f);
    if (diff > 0.0) {
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * (spec * texture2D(material.specular, TexCoord).rgb);
    }

//    vec3 emission = texture(material.emission, TexCoord).rgb;
    vec3 emission = vec3(0.0f);
//    if (texture2D(material.specular, TexCoord).r < 0.1){
//        emission = texture2D(material.emission, TexCoord).rgb;
//    }

    vec3 result = (ambient + diffuse + specular) * attenuation + emission;
    FragColor = vec4(result, 1.0);
}