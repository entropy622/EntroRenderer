#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef> // 用于 offsetof

// 1. 定义一个标准的顶点结构体
// 以后如果你想加法线(Normal)，直接在这里加 glm::vec3 Normal; 
// 下面的步长计算会自动更新，不用你去改代码！
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    // 网格数据
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    // 构造函数
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;

        setupMesh(); // 自动设置
    }

    // 绘制函数
    void Draw(unsigned int shaderID) {
        glBindVertexArray(VAO);
        // 如果有索引就用索引画，没有就直接画数组（通常都用索引）
        if (!indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
        }
        glBindVertexArray(0);
    }

    // 析构函数（可选，简单起见先不写复杂的资源管理）
    // ~Mesh() { glDeleteVertexArrays(1, &VAO); ... }

private:
    unsigned int VBO, EBO;

    void setupMesh() {
        // 创建 ID
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 1. 绑定并填充 VBO
        // 技巧：利用结构体的内存布局直接传数据
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // 2. 绑定并填充 EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 3. 设置顶点属性指针 (最通用的部分！)
        
        // 属性 0: 位置 (Position)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // 属性 1: 纹理坐标 (TexCoords)
        // 魔法解释：offsetof(Vertex, TexCoords) 会自动计算 TexCoords 在结构体里的字节偏移量
        // 你再也不用手算 "3 * 4 = 12" 了！
        glEnableVertexAttribArray(1); 
        // 注意：这里我把它设为了 Location 1。请同步修改你的 Shader (layout=1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};

#endif