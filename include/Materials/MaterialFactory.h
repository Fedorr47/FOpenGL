
#pragma once
#include <memory>
#include "Materials/Material.h"

namespace MaterialFactory {
    inline std::shared_ptr<Material> CreateShiny() { return std::make_shared<Material>(1.0f, 64.0f); }
    inline std::shared_ptr<Material> CreateDull()  { return std::make_shared<Material>(0.3f, 8.0f); }
}
