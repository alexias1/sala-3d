#include <glad/glad.h>   // SEMPRE antes do GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"

const int SCR_W = 1280, SCR_H = 720;

Camera camera;
float lastX = SCR_W / 2.0f, lastY = SCR_H / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;
bool lampOn = true;

void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos;
    camera.rotate(dx, dy);
}

// chamada UMA vez por aperto de tecla (bom para ligar/desligar coisas)
void key_callback(GLFWwindow*, int key, int, int action, int) {
    if (key == GLFW_KEY_L && action == GLFW_PRESS) lampOn = !lampOn;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.moveForward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.moveBackward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.moveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.moveRight(deltaTime);
}

// Envia material + textura + matriz model e desenha a malha
void drawObj(Shader& s, const Mesh& mesh, const glm::mat4& model, const Material& m,
             unsigned int tex, glm::vec2 uvScale = glm::vec2(1.0f)) {
    s.setMat4("model", model);
    s.setVec3("material.ka", m.ka);
    s.setVec3("material.kd", m.kd);
    s.setVec3("material.ks", m.ks);
    s.setFloat("material.shininess", m.shininess);
    s.setVec3("material.emissive", m.emissive);
    s.setVec2("uvScale", uvScale);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    mesh.draw();
}

// Atalho: cria matriz model = translate * rotate(Y) * scale
glm::mat4 makeModel(glm::vec3 pos, glm::vec3 scale, float rotY = 0.0f) {
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(rotY), glm::vec3(0, 1, 0));
    m = glm::scale(m, scale);
    return m;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "Sala 3D", nullptr, nullptr);
    if (!window) { std::cerr << "Falha ao criar janela\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "Falha ao iniciar GLAD\n"; return -1; }
    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/phong.vert", "shaders/phong.frag");

    // Malhas (criadas uma vez, desenhadas várias vezes com model diferente)
    Mesh cube = Mesh::cube();
    Mesh plane = Mesh::plane();
    Mesh cylinder = Mesh::cylinder();
    Mesh sphere = Mesh::sphere();

    // Texturas (se o arquivo não existir, usa branco e avisa no terminal)
    unsigned int white = makeWhiteTexture();
    unsigned int texWood = loadTexture("assets/textures/wood.jpg");
    unsigned int texFloor = loadTexture("assets/textures/floor.jpg");
    unsigned int texWall = loadTexture("assets/textures/wall.jpg");

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;
        processInput(window);

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        shader.setMat4("projection", glm::perspective(glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
        shader.setMat4("view", camera.getViewMatrix());
        shader.setVec3("viewPos", camera.position);
        shader.setInt("tex", 0);

        // ---- Luzes ----
        glm::vec3 lampPos(0.0f, 1.3f, 0.0f);
        shader.setInt("numLights", 2);
        shader.setVec3("lights[0].position", lampPos);
        shader.setVec3("lights[0].color", lampOn ? glm::vec3(1.0f, 0.85f, 0.6f) * 1.5f : glm::vec3(0.0f));
        shader.setFloat("lights[0].linear", 0.09f);
        shader.setFloat("lights[0].quadratic", 0.032f);
        shader.setVec3("lights[1].position", glm::vec3(0.0f, 2.8f, 2.0f));
        shader.setVec3("lights[1].color", glm::vec3(0.3f, 0.3f, 0.4f));
        shader.setFloat("lights[1].linear", 0.09f);
        shader.setFloat("lights[1].quadratic", 0.032f);

        // ---- Cenário ----
        drawObj(shader, plane, makeModel({0, 0, 0}, {10, 1, 10}), Mat::wood(), texFloor, {5, 5});
        drawObj(shader, cube, makeModel({0, 1.5f, -5}, {10, 3, 0.2f}), Mat::wall(), texWall, {3, 1});
        drawObj(shader, cube, makeModel({-5, 1.5f, 0}, {0.2f, 3, 10}), Mat::wall(), texWall, {3, 1});

        // ---- Mesa: tampo + 4 pernas ----
        drawObj(shader, cube, makeModel({0, 0.75f, 0}, {1.6f, 0.1f, 0.9f}), Mat::wood(), texWood);
        float lx[] = {-0.7f, 0.7f}, lz[] = {-0.35f, 0.35f};
        for (float x : lx) for (float z : lz)
            drawObj(shader, cube, makeModel({x, 0.35f, z}, {0.1f, 0.7f, 0.1f}), Mat::wood(), texWood);

        // ---- Abajur: base + haste + lâmpada ----
        drawObj(shader, cylinder, makeModel({0, 0.825f, 0}, {0.25f, 0.05f, 0.25f}), Mat::metal(), white);
        drawObj(shader, cylinder, makeModel({0, 1.05f, 0}, {0.04f, 0.4f, 0.04f}), Mat::metal(), white);
        glm::vec3 glow = lampOn ? glm::vec3(1.0f, 0.9f, 0.6f) : glm::vec3(0.0f);
        drawObj(shader, sphere, makeModel({0, 1.3f, 0}, {0.2f, 0.2f, 0.2f}), Mat::glowing(glow), white);

        // ---- Esfera de metal girando sobre a mesa ----
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.95f, 0.0f));
        m = glm::rotate(m, now, glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(0.3f));
        drawObj(shader, sphere, m, Mat::metal(), white);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}