//
// Created by Aentro on 2025/12/14.
//

#ifndef LEARNOPENGL_CLION_POINTLIGHTDATA_H
#define LEARNOPENGL_CLION_POINTLIGHTDATA_H
// 强烈建议使用 vec4 替代 vec3 + padding，避免编译器对齐差异
struct PointLightData {
    glm::vec4 position;  // xyz = position, w = padding
    glm::vec4 ambient;   // xyz = ambient,  w = padding
    glm::vec4 diffuse;   // xyz = diffuse,  w = padding
    glm::vec4 specular;  // xyz = specular, w = padding

    // 最后的衰减参数
    float constant;
    float linear;
    float quadratic;
    float padding; // 补齐到 16 字节 (4 floats)
};
#endif //LEARNOPENGL_CLION_POINTLIGHTDATA_H