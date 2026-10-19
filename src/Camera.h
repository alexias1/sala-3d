#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    float yaw = -90.0f;   // giro horizontal (graus)
    float pitch = 0.0f;   // giro vertical (graus)
    float speed = 3.0f;
    float sensitivity = 0.1f;

    Camera(glm::vec3 pos = glm::vec3(0.0f, 1.6f, 5.0f)) : position(pos) { updateVectors(); }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    // Move só no plano horizontal (estilo FPS: olhar pra cima não faz voar)
    void moveForward(float dt)  { position += glm::normalize(glm::vec3(front.x, 0, front.z)) * speed * dt; }
    void moveBackward(float dt) { position -= glm::normalize(glm::vec3(front.x, 0, front.z)) * speed * dt; }
    void moveRight(float dt)    { position += right * speed * dt; }
    void moveLeft(float dt)     { position -= right * speed * dt; }

    void rotate(float dx, float dy) {
        yaw   += dx * sensitivity;
        pitch += dy * sensitivity;
        if (pitch > 89.0f)  pitch = 89.0f;   // evita virar de cabeça pra baixo
        if (pitch < -89.0f) pitch = -89.0f;
        updateVectors();
    }

private:
    void updateVectors() {
        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(f);
        right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
        up = glm::normalize(glm::cross(right, front));
    }
};