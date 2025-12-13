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
#include <assimp/scene.h> // 依然需要 scene.h 来识别 aiNode 等指针，但不需要 Importer

using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
    vector<TextureInfo> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    Model(string const &path, bool gamma = false);
    void Draw(Shader &shader);

private:
    void loadModel(string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    vector<TextureInfo> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif