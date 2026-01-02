#pragma once
#include <memory>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec3.hpp>

class Mesh;
class Texture;

struct ModelTransform {
    glm::vec3 pos{0.0f};
    glm::vec3 rotDeg{0.0f};
    glm::vec3 scale{1.0f};
};

class Model
{
public:
    Model();
    ~Model();
    Model(const Model& other){}
    Model& operator=(const Model& other){}
    Model(Model&& other){}
    Model& operator=(Model&& other){}

    void LoadModel(const char* filename);
    void Render();
    void Release();

    void SetPosition(glm::vec3 position) {transform_.pos = position;}
    void SetRotation(glm::vec3 rotation){ transform_.rotDeg = rotation;}
    void SetScale(glm::vec3 scale){ transform_.scale = scale;}

    glm::vec3 GetTranslation() const {return transform_.pos;}
    glm::vec3 GetRotation() const {return transform_.rotDeg;}
    glm::vec3 GetScale() const {return transform_.scale;}
  
private:
    
    void LoadModel(aiNode* node, const aiScene* scene);
    void LoadMesh(aiMesh* mesh, const aiScene* scene);
    void LoadMaterials(const aiScene* scene);
  
    // TODO: change to id map against this 3 vectors
    std::vector<std::shared_ptr<Mesh>> _meshList;
    std::vector<std::shared_ptr<Texture>> _textureList;
    std::vector<unsigned int> _meshToTexture;

    ModelTransform transform_;
};