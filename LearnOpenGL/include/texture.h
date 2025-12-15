#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <iostream>
#include "stb_image.h" // 确保这一行能找到你的 stb_image.h

class Texture {
public:
    unsigned int ID;
    unsigned int type;
    int width, height, nrChannels;

    // 构造函数：传入文件路径
    // ------------------------------------------------------------------------
    Texture(const char* path, int wrapping = GL_REPEAT) {
        type = GL_TEXTURE_2D; // 默认为 2D 纹理

        glGenTextures(1, &ID);
        glBindTexture(type, ID); // 先绑定，后续的操作都是针对这个ID的

        // 设置纹理环绕方式 (Wrapping)
        // --------------------------------------------------------------------
        glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapping); // S轴 (x) 重复
        glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapping); // T轴 (y) 重复

        // 设置纹理过滤方式 (Filtering)
        // --------------------------------------------------------------------
        // 缩小(Minify)时使用 Mipmap 线性过滤，放大(Magnify)时使用线性插值
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 加载图片数据
        // --------------------------------------------------------------------
        unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

        if (data) {
            // 自动判断图片格式 (JPG通常是RGB, PNG通常是RGBA)
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            // 生成纹理
            glTexImage2D(type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(type); // 自动生成多级渐远纹理
        }
        else {
            std::cout << "Failed to load texture: " << path << std::endl;
        }

        // 释放内存
        stbi_image_free(data);
    }

    // 激活并绑定纹理到指定的纹理单元 (Slot)
    // slot = 0 对应 GL_TEXTURE0, slot = 1 对应 GL_TEXTURE1
    // ------------------------------------------------------------------------
    void bind(unsigned int slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(type, ID);
    }
};

#endif