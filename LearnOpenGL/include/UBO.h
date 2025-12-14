#ifndef UBO_H
#define UBO_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class UBO {
public:
    unsigned int ID;

    // 构造函数：分配内存
    // size: 缓冲大小 (2个 mat4 = 2 * 64 = 128 字节)
    // bindingPoint: 绑定点 (我们约定用 0)
    UBO(unsigned int size, unsigned int bindingPoint = 0) {
        glGenBuffers(1, &ID);
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        
        // 分配内存，但不填数据 (NULL)，使用 GL_STATIC_DRAW 因为矩阵每帧只变一次
        glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
        
        // 将这个 Buffer 绑定到绑定点 (Binding Point)
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ID);
        
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    ~UBO() {
        glDeleteBuffers(1, &ID);
    }

    // 写入数据
    // offset: 偏移量 (字节)
    // size: 数据大小
    // data: 数据指针
    void SetData(unsigned int offset, unsigned int size, const void* data) {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    
    // 专门为矩阵提供的快捷函数
    void SetMat4(unsigned int offset, const glm::mat4& mat) {
        SetData(offset, sizeof(glm::mat4), glm::value_ptr(mat));
    }
};

#endif