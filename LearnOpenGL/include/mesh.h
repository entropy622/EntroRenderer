#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef> // 必须包含，为了使用 offsetof

// 1. 定义标准的顶点结构体
// 这里的内存布局是：[float, float, float] [float, float]
// 总大小自动为 5 * 4 = 20 字节
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    // 构造函数：接收顶点和索引
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}) {
        this->vertices = vertices;
        this->indices = indices;
        setupMesh();
    }

    // 绘制函数
    void Draw() {
        glBindVertexArray(VAO);
        if (!indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            // 如果没有索引，就直接画数组（比如这次的立方体）
            glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
        }
        glBindVertexArray(0);
    }

    // 析构清理
    void Delete() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

private:
    unsigned int VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 1. 填充 VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // 【核心魔法】直接用结构体数组的指针
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // 2. 填充 EBO (如果有)
        if (!indices.empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        }

        // 3. 设置属性指针 (通用化)
        // 属性 0: Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // 属性 1: TexCoords
        // offsetof 会自动计算 TexCoords 距离结构体开头隔了多少字节
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};

#endif