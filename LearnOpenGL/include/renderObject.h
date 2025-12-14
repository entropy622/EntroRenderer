#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include "model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class RenderObject {
public:
    // 【改进 1】使用指针！
    // 这样 100 个物体可以共用同一个 Model 实例，省内存
    Model* model; 
    
    // 【改进 2】增加手动纹理支持
    // 如果模型自带纹理，这个设为 0；如果是白模，这里填 loadTexture 返回的 ID
    unsigned int textureID; 

    // 变换属性
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // 构造函数
    // 传入已经加载好的 Model 指针，而不是路径
    RenderObject(Model* modelPtr, unsigned int texID = 0) 
        : model(modelPtr), textureID(texID) 
    {
        // 初始化默认值
        position = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        scale    = glm::vec3(1.0f); // ⚠️ 默认缩放必须是 1，否则看不见！
    }

    // 【改进 3】Shader 作为参数传入
    // 这样你可以用同一个 Shader 画不同的物体（批处理思想）
    void Draw(Shader& shader) {
        // 1. 如果有手动设置的纹理，先绑定
        if (textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            // shader.setInt("material.diffuse", 0); // 确保 Shader 读取 0 号位
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
        shader.use();
        shader.setMat4("model", modelMat);

        // 4. 绘制
        model->Draw(shader);
    }
};

#endif