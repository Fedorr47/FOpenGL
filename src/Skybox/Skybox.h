#pragma once

#include <memory>
#include <vector>
#include <string>

#include <Gl\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

class Mesh;
class Shader;

class Skybox
{
public:
    Skybox(){}
    ~Skybox(){}
    Skybox(const Skybox& skybox){}
    Skybox& operator=(const Skybox& skybox){}
    Skybox(Skybox&& skybox){}
    Skybox& operator=(Skybox&& skybox){}

    explicit Skybox(const std::vector<std::string>& facesLocations);

    void Draw(glm::mat4 viewMatrix, glm::mat4 projMatrix) const;
    
private:
    std::unique_ptr<Shader> skyShader_;
    std::unique_ptr<Mesh> skyMesh_;

    GLuint skyTextureId_;
    GLuint uniformProjection_, uniformView_;
};
