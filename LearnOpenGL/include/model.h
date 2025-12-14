#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 注意：这里不再包含 assimp 的头文件了！只包含 Mesh.h 和 Shader.h
#include "mesh.h"
#include "shader.h"

#include <string>
#include <vector>

#include "assimp/material.h"

using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
class Model
{
public:
    vector<TextureInfo> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    Model(string const &path, bool gamma = false);
    void Draw(Shader &shader);
    void DrawAt(glm::vec3 pos, Shader &shader);

private:
    void loadModel(string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    vector<TextureInfo> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif