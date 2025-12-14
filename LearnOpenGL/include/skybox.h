#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "stb_image.h" // 确保你的 include 目录里有这个

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Skybox {
public:
    // 构造函数：传入 6 张图片的路径
    // 顺序必须是: Right, Left, Top, Bottom, Front, Back
    Skybox(const vector<string>& faces) {
        setupMesh();
        cubemapTexture = loadCubemap(faces);
    }

    // 析构函数：清理 GPU 资源
    ~Skybox() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteTextures(1, &cubemapTexture);
    }

    // 绘制函数
    // 只需要传入 shader 和当前的 view/projection 矩阵
    // 内部会自动处理 View 矩阵移除位移、深度测试模式切换等逻辑
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
        // 1. 改变深度测试条件为 LEQUAL (小于等于)
        // 这是一个优化：天空盒深度永远是 1.0，如果不改 LEQUAL 可能会被其他物体挡住或画不出来
        glDepthFunc(GL_LEQUAL);
        
        shader.use();

        // 2. 移除 View 矩阵的位移部分 (只保留旋转)
        // 这样天空盒才会让人感觉是“无限远”的，无论你怎么移动摄像机，天空都不会变近
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view)); 
        
        shader.setMat4("view", viewNoTranslation);
        shader.setMat4("projection", projection);

        // 3. 绘制立方体
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // 4. 恢复默认深度测试条件
        glDepthFunc(GL_LESS);
    }

private:
    unsigned int VAO, VBO;
    unsigned int cubemapTexture;

    // 初始化立方体网格
    void setupMesh() {
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    // 加载 Cubemap 纹理
    unsigned int loadCubemap(const vector<string>& faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++) {
            // stbi_load 不需要 #define STB_IMAGE_IMPLEMENTATION，因为 Model.cpp 里已经定义过了
            // 如果链接报错，请检查是否有一个 cpp 定义了该宏
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                // 这里的格式根据图片通道数自动判断，防止 jpg/png 混合加载时出错
                GLenum format = GL_RGB;
                if (nrChannels == 4) format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                             0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            } else {
                cout << "Cubemap texture failed to load at path: " << faces[i] << endl;
                stbi_image_free(data);
            }
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }
};

#endif