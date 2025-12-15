#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include "model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class RenderObject {
public:
    Model* model; 

    // 如果模型自带纹理，这个设为 0；
    unsigned int textureID;
    unsigned int normalMapID;

    // 变换属性
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec2 uvScale;

    // 构造函数
    // 传入已经加载好的 Model 指针，而不是路径
    RenderObject(Model* modelPtr, unsigned int texID = 0, unsigned int normalMapID = 0)
        : model(modelPtr), textureID(texID), normalMapID(normalMapID)
    {
        // 初始化默认值
        position = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        scale    = glm::vec3(1.0f); // ⚠️ 默认缩放必须是 1，否则看不见！
        uvScale  = glm::vec2(1.0f);
    }

    // 【改进 3】Shader 作为参数传入
    // 这样你可以用同一个 Shader 画不同的物体（批处理思想）
    void Draw(Shader& shader) {
        // 1. 如果有手动设置的纹理，先绑定
        shader.use();
        if (textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            shader.setInt("material.texture_diffuse1", 0);
        }
        if (normalMapID != 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalMapID);

            shader.setInt("material.texture_normal1", 1);

            // 启用法线贴图开关
            shader.setBool("useNormalMap", true);
        } else {
            shader.setBool("useNormalMap", false);
        }

        // 2. 计算矩阵
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, position);
        // 欧拉角旋转 (注意顺序: Z -> Y -> X 或 X -> Y -> Z，这里分开写没问题)
        if(rotation.x != 0) modelMat = glm::rotate(modelMat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        if(rotation.y != 0) modelMat = glm::rotate(modelMat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        if(rotation.z != 0) modelMat = glm::rotate(modelMat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMat = glm::scale(modelMat, scale);

        // 3. 设置 Uniform
        shader.setVec2("uvScale", uvScale);
        shader.setMat4("model", modelMat);

        // 4. 绘制
        model->Draw(shader);
    }
};

#endif