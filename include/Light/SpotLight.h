#pragma once
#include "PointLight.h"
#include "Light/LightProperties.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

class SpotLight : public PointLightBase<SpotLight>
{
public:
    SpotLight(const PropsType& p)
    {
        SetProperties(p);
    }
    
    void UseLight(const UniformType& u) const override
    {
        Mixin::UseLight(u);

        const auto& Props = this->GetLightProperties();

        if (u.Position  != (GLuint)-1)
            glUniform3f(u.Position, Props.Position.x, Props.Position.y, Props.Position.z);
        if (u.Constant  != (GLuint)-1)
            glUniform1f(u.Constant, Props.Constant);
        if (u.Linear    != (GLuint)-1)
            glUniform1f(u.Linear,   Props.Linear);
        if (u.Exponent  != (GLuint)-1)
            glUniform1f(u.Exponent, Props.Exponent);
        
        const glm::vec3 d = hasDir_ ? normDir_ : glm::vec3(0.0f, -1.0f, 0.0f);
        if (u.Direction != (GLuint)-1)
            glUniform3f(u.Direction, d.x, d.y, d.z);
        if (u.Edge      != (GLuint)-1)
            glUniform1f(u.Edge, Props.ProcEdge);
    }

    void SetProperties(const SpotLightProperties& p) override
    {
        PointLightBase<SpotLight>::SetProperties(p);
        auto& Props = Mixin::GetLightProperties();
        
        Props.Direction = p.Direction;
        Props.Edge = p.Edge;
        Props.ProcEdge = std::cos(glm::radians(p.Edge));

        if (Props.Direction.length() > 0.0f)
        {
            hasDir_  = true;
        }
    }

    void SetFlash(glm::vec3 inPosition, glm::vec3 inDirection)
    {
        auto& Props = Mixin::GetLightProperties();
        Props.Position = inPosition;
        Props.Direction = inDirection;

        Props.ProcEdge = std::cos(glm::radians(Props.Edge));
        normDir_  = glm::normalize(Props.Direction);
        if (Props.Direction.length() > 0.0f)
        {
            hasDir_  = true;
        }
    }

    glm::vec3 normDir_{0.0f, -1.0f, 0.0f};
    bool hasDir_{false};
};

