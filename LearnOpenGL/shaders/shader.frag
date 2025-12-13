#version 330 core
out vec4 FragColor;

struct Material {
    vec3 diffuse;   // 漫反射颜色 (通常等于物体颜色)
    vec3 specular;  // 镜面光颜色 (高光的颜色)
    float shininess; // 反光度 (高光点的聚焦程度)
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    vec4 textureColor = texture(texture1, TexCoord);
    vec4 textureColor2 = texture(texture2, TexCoord);
    vec3 baseColor = mix(textureColor, textureColor2, 0.8).rgb;
    // 1. 环境光
    vec3 ambient = light.ambient * baseColor;

    // 2. 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // 3. 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}