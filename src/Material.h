#pragma once
#include <glm/glm.hpp>

struct Material {
    glm::vec3 ka, kd, ks;
    float shininess;
    glm::vec3 emissive{0.0f};
};

namespace Mat {
    inline Material wood()    { return {{0.25f,0.25f,0.25f}, {0.8f,0.8f,0.8f}, {0.10f,0.10f,0.10f}, 8.0f}; }
    inline Material metal()   { return {{0.20f,0.20f,0.20f}, {0.4f,0.4f,0.4f}, {1.00f,1.00f,1.00f}, 128.0f}; }
    inline Material plastic() { return {{0.20f,0.20f,0.20f}, {0.7f,0.7f,0.7f}, {0.50f,0.50f,0.50f}, 32.0f}; }
    inline Material wall()    { return {{0.30f,0.30f,0.30f}, {0.8f,0.8f,0.8f}, {0.02f,0.02f,0.02f}, 2.0f}; }
    inline Material glowing(glm::vec3 c) { return {{0.2f,0.2f,0.2f}, {0.5f,0.5f,0.5f}, {0.3f,0.3f,0.3f}, 16.0f, c}; }
}