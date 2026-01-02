#pragma once

static void ComputeAverageNormals(
    std::vector<float>& v, const std::vector<unsigned>& idx,
    unsigned strideFloats = 8, unsigned normalOffset = 5)
{
    for (size_t i=0;i<v.size()/strideFloats;++i) {
        v[i*strideFloats + normalOffset + 0] = 0.f;
        v[i*strideFloats + normalOffset + 1] = 0.f;
        v[i*strideFloats + normalOffset + 2] = 0.f;
    }

    for (size_t t=0;t<idx.size(); t+=3) {
        auto i0 = idx[t+0], i1 = idx[t+1], i2 = idx[t+2];
        auto p0 = glm::vec3(v[i0*strideFloats+0], v[i0*strideFloats+1], v[i0*strideFloats+2]);
        auto p1 = glm::vec3(v[i1*strideFloats+0], v[i1*strideFloats+1], v[i1*strideFloats+2]);
        auto p2 = glm::vec3(v[i2*strideFloats+0], v[i2*strideFloats+1], v[i2*strideFloats+2]);
        glm::vec3 n = glm::normalize(glm::cross(p1-p0, p2-p0));
        for (auto i : {i0,i1,i2}) {
            v[i*strideFloats+normalOffset+0] += n.x;
            v[i*strideFloats+normalOffset+1] += n.y;
            v[i*strideFloats+normalOffset+2] += n.z;
        }
    }

    for (size_t i=0;i<v.size()/strideFloats;++i) {
        glm::vec3 n(v[i*strideFloats+normalOffset+0],
                    v[i*strideFloats+normalOffset+1],
                    v[i*strideFloats+normalOffset+2]);
        n = glm::normalize(n);
        v[i*strideFloats+normalOffset+0] = n.x;
        v[i*strideFloats+normalOffset+1] = n.y;
        v[i*strideFloats+normalOffset+2] = n.z;
    }
}

namespace LightUtils
{
    
    static DirectionalLightProperties get_default_direct_light_properties()
    {
        DirectionalLightProperties dlp{};
        dlp.Colour = {1,1,1};
        dlp.AmbientIntensity = 0.1f;
        dlp.DiffuseIntensity = 0.6f;
        dlp.Direction = { 0.0f, -7.0f, -5.0f };
        dlp.shadowMapPtr->Initialize(2048, 2048);
        
        return dlp;
    }
    
    static PointLightProperties get_default_point_light_properties()
    {
        PointLightProperties p{};
        p.Colour = {1.0f, 1.0f, 1.0f};
        p.AmbientIntensity = 0.1f;
        p.DiffuseIntensity = 1.0f;
        p.Position = {0.0f, 1.0f, 0.0f};
        p.Constant = 1.0f;
        p.Linear   = 0.09f;
        p.Exponent = 0.032f;
        
        p.nearPlane = 0.01f;
        p.farPlane  = 100.0f;
        if (p.shadowMapPtr)
        {
            p.shadowMapPtr->Initialize(1024, 1024);
        }
        return p;
    }

    static SpotLightProperties get_default_spot_light_properties()
    {
        SpotLightProperties p{};
        p.Colour = {1,1,1};
        p.AmbientIntensity = 0.1f;
        p.DiffuseIntensity = 1.0f;

        p.Position = {0.f, 5.f, -2.5f};
        p.Direction = {0.f, -1.f, 0.f};
        p.Edge = 20.0f;

        p.Constant = 1.0f;
        p.Linear   = 0.09f;
        p.Exponent = 0.032f;
        p.nearPlane = 0.01f;
        p.farPlane  = 100.0f;

        if (p.shadowMapPtr) {
            p.shadowMapPtr->Initialize(1024, 1024);
        }

        return p;
    }
}