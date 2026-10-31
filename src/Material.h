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

    // ---------- Halloween ----------
    // Parede arroxeada e escura (a textura ainda aparece, só que tingida)
    inline Material wallHalloween() { return {{0.14f,0.09f,0.20f}, {0.55f,0.42f,0.70f}, {0.15f,0.15f,0.15f}, 16.0f}; }
    // Chão de madeira escurecida
    inline Material floorDark()     { return {{0.12f,0.10f,0.12f}, {0.50f,0.40f,0.45f}, {0.25f,0.25f,0.25f}, 32.0f}; }
    inline Material pumpkin() { return {{0.30f,0.12f,0.02f}, {0.95f,0.42f,0.05f}, {0.25f,0.20f,0.10f}, 24.0f}; }
    inline Material stem()    { return {{0.10f,0.15f,0.03f}, {0.28f,0.42f,0.10f}, {0.05f,0.05f,0.05f}, 4.0f}; }
    inline Material bone()    { return {{0.30f,0.30f,0.25f}, {0.85f,0.82f,0.70f}, {0.20f,0.20f,0.20f}, 16.0f}; }
    inline Material wax()     { return {{0.30f,0.30f,0.28f}, {0.90f,0.88f,0.80f}, {0.30f,0.30f,0.30f}, 16.0f}; }
    inline Material black()   { return {{0.03f,0.03f,0.03f}, {0.05f,0.05f,0.05f}, {0.30f,0.30f,0.30f}, 32.0f}; }
    inline Material stone()   { return {{0.15f,0.15f,0.17f}, {0.45f,0.45f,0.50f}, {0.05f,0.05f,0.05f}, 4.0f}; }
    // Teia de aranha: levemente "autoiluminada" para aparecer no escuro
    inline Material web()     { return {{0.50f,0.50f,0.50f}, {0.60f,0.60f,0.60f}, {0.0f,0.0f,0.0f}, 1.0f, {0.18f,0.18f,0.22f}}; }
}