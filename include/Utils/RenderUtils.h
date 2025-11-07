#pragma once

static void CalculateAverageNormals(
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