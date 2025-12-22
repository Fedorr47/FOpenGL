#include "Skybox/Skybox.h"

#include "Model/Mesh.h"
#include "Rendering/Shader.h"
#include "ThirdParty/stb_image.h"

Skybox::Skybox(const std::vector<std::string>& facesLocations)
{
    skyShader_ = std::make_unique<Shader>();
    skyMesh_ = std::make_unique<Mesh>();

    skyShader_->CreateFromFiles("shaders/skybox.vert", "shaders/skybox.frag");
    uniformProjection_ = skyShader_->GetUniformProj();
    uniformView_ = skyShader_->GetUniformView();

    glGenTextures(1, &skyTextureId_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyTextureId_);

    int width, height, bitDepth;

    for (size_t i = 0; i < 6; i++)
    {
        unsigned char* data = stbi_load(facesLocations[i].c_str(), &width, &height, &bitDepth, 0);
        if (!data)
        {
            return;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Mesh Setup
    std::vector<unsigned int> skyboxIndices = {
        // front
        0, 1, 2,
        2, 1, 3,
        // right
        2, 3, 5,
        5, 3, 7,
        // back
        5, 7, 4,
        4, 7, 6,
        // left
        4, 6, 0,
        0, 6, 1,
        // top
        4, 0, 5,
        5, 0, 2,
        // bottom
        1, 6, 3,
        3, 6, 7
    };

    std::vector<float> skyboxVertices = {
        -1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,

        -1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f
    };
    
    skyMesh_->Create(skyboxVertices, skyboxIndices);
}

void Skybox::Draw(glm::mat4 viewMatrix, glm::mat4 projMatrix) const
{
    viewMatrix = glm::mat4(glm::mat3(viewMatrix));

    glDepthMask(GL_FALSE);

    skyShader_->Use();

    glUniformMatrix4fv(uniformProjection_, 1, GL_FALSE, glm::value_ptr(projMatrix));
    glUniformMatrix4fv(uniformView_, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // TODO: Maybe it should be change later to some other texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyTextureId_);

    skyMesh_->Draw();

    glDepthMask(GL_TRUE);
}
