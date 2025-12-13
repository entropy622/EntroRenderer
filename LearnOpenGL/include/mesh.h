#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "shader.h"

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct TextureInfo {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // 网格数据
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<TextureInfo>  textures;
    unsigned int VAO;

    // 构造函数
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<TextureInfo> textures);

    // 绘制函数
    void Draw(Shader &shader);

private:
    unsigned int VBO, EBO;
    void setupMesh();
};
#endif