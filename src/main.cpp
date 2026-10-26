#include <glad/glad.h>   // SEMPRE antes do GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"

const int SCR_W = 1280, SCR_H = 720;

Camera camera(glm::vec3(0.0f, 1.6f, 3.5f));
float lastX = SCR_W / 2.0f, lastY = SCR_H / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

// ---------- Estado dos objetos interativos ----------
bool lampOn = true, tvOn = false, fanOn = false, doorTarget = false;
float doorOpen = 0.0f;    // 0 = fechada, 1 = aberta (animado aos poucos)
float fanSpeed = 0.0f;    // graus por segundo (acelera e desacelera)
float fanAngle = 0.0f;

struct Interactable { const char* name; glm::vec3 pos; bool* state; };
Interactable items[] = {
    {"porta",      {5.0f, 1.0f, 1.5f},   &doorTarget},
    {"TV",         {2.5f, 1.2f, -4.8f},  &tvOn},
    {"ventilador", {-2.5f, 2.8f, -2.5f}, &fanOn},
    {"abajur",     {0.0f, 1.3f, 0.0f},   &lampOn}
};
const int NITEMS = 4;
const float REACH = 2.5f;   // alcance (metros)

// Objeto interativo mais próximo da câmera (distância no plano XZ), ou -1
int nearestItem() {
    int best = -1; float bestD = REACH;
    for (int i = 0; i < NITEMS; i++) {
        float d = glm::length(glm::vec2(camera.position.x - items[i].pos.x,
                                        camera.position.z - items[i].pos.z));
        if (d < bestD) { bestD = d; best = i; }
    }
    return best;
}

void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos;
    camera.rotate(dx, dy);
}

void key_callback(GLFWwindow*, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_E) {
        int i = nearestItem();
        if (i >= 0) *items[i].state = !*items[i].state;   // alterna o estado
    }
}

void processInput(GLFWwindow* window) {
    auto down = [&](int k1, int k2) {
        return glfwGetKey(window, k1) == GLFW_PRESS || glfwGetKey(window, k2) == GLFW_PRESS;
    };
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (down(GLFW_KEY_W, GLFW_KEY_UP))    camera.moveForward(deltaTime);
    if (down(GLFW_KEY_S, GLFW_KEY_DOWN))  camera.moveBackward(deltaTime);
    if (down(GLFW_KEY_A, GLFW_KEY_LEFT))  camera.moveLeft(deltaTime);
    if (down(GLFW_KEY_D, GLFW_KEY_RIGHT)) camera.moveRight(deltaTime);
}

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

glm::mat4 makeModel(glm::vec3 pos, glm::vec3 scale, float rotY = 0.0f) {
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(rotY), glm::vec3(0, 1, 0));
    m = glm::scale(m, scale);
    return m;
}

// Move "v" em direção a "alvo" com velocidade "vel" (por segundo), sem passar do alvo
float approach(float v, float alvo, float vel) {
    if (v < alvo) return std::min(alvo, v + vel);
    return std::max(alvo, v - vel);
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

    Mesh cube = Mesh::cube();
    Mesh plane = Mesh::plane();
    Mesh cylinder = Mesh::cylinder();
    Mesh sphere = Mesh::sphere();

    unsigned int white = makeWhiteTexture();
    unsigned int texWood = loadTexture("assets/textures/wood.jpg");
    unsigned int texFloor = loadTexture("assets/textures/floor.jpg");
    unsigned int texWall = loadTexture("assets/textures/wall.jpg");

    // Materiais extras da TV
    Material tvFrame{{0.05f,0.05f,0.05f}, {0.1f,0.1f,0.1f}, {0.3f,0.3f,0.3f}, 32.0f};
    Material tvOff  {{0.02f,0.02f,0.02f}, {0.03f,0.03f,0.03f}, {0.8f,0.8f,0.8f}, 96.0f};

    std::string lastTitle;

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        // ---------- Entrada + colisão ----------
        glm::vec3 old = camera.position;
        processInput(window);
        glm::vec3 np = camera.position;
        // caixa da mesa (com folga de 0.3 m)
        auto blocked = [](float x, float z) { return std::fabs(x) < 1.1f && std::fabs(z) < 0.75f; };
        glm::vec3 p = old;
        p.x = np.x; if (blocked(p.x, p.z)) p.x = old.x;   // testa eixo X
        p.z = np.z; if (blocked(p.x, p.z)) p.z = old.z;   // depois eixo Z (permite deslizar)
        p.x = std::clamp(p.x, -4.7f, 4.7f);               // limites da sala
        p.z = std::clamp(p.z, -4.7f, 4.7f);
        p.y = np.y;
        camera.position = p;

        // ---------- Animações ----------
        doorOpen = approach(doorOpen, doorTarget ? 1.0f : 0.0f, deltaTime * 1.2f);
        float s = doorOpen * doorOpen * (3.0f - 2.0f * doorOpen);   // smoothstep: começa e termina suave
        float doorAngle = -95.0f * s;                               // negativo = abre para dentro da sala
        fanSpeed += ((fanOn ? 400.0f : 0.0f) - fanSpeed) * std::min(1.0f, deltaTime * 1.5f);
        fanAngle += fanSpeed * deltaTime;

        // ---------- Dica no título da janela ----------
        std::string title = "Sala 3D";
        int ni = nearestItem();
        if (ni >= 0)
            title += std::string("   |   [E] ") + items[ni].name +
                     (*items[ni].state ? " (desligar/fechar)" : " (ligar/abrir)");
        if (title != lastTitle) { glfwSetWindowTitle(window, title.c_str()); lastTitle = title; }

        // ---------- Desenho ----------
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        shader.setMat4("projection", glm::perspective(glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
        shader.setMat4("view", camera.getViewMatrix());
        shader.setVec3("viewPos", camera.position);
        shader.setInt("tex", 0);

        // ---- Luzes ----
        float flicker = 0.85f + 0.15f * std::sin(now * 9.0f);   // TV "pisca" levemente
        shader.setInt("numLights", 3);
        shader.setVec3("lights[0].position", glm::vec3(0.0f, 1.3f, 0.0f));   // abajur
        shader.setVec3("lights[0].color", lampOn ? glm::vec3(1.0f, 0.85f, 0.6f) * 1.5f : glm::vec3(0.0f));
        shader.setFloat("lights[0].linear", 0.09f);
        shader.setFloat("lights[0].quadratic", 0.032f);
        shader.setVec3("lights[1].position", glm::vec3(0.0f, 2.8f, 2.0f));  // luz fraca ambiente
        shader.setVec3("lights[1].color", glm::vec3(0.3f, 0.3f, 0.4f));
        shader.setFloat("lights[1].linear", 0.09f);
        shader.setFloat("lights[1].quadratic", 0.032f);
        shader.setVec3("lights[2].position", glm::vec3(2.5f, 1.2f, -4.0f)); // TV
        shader.setVec3("lights[2].color", tvOn ? glm::vec3(0.2f, 0.45f, 1.0f) * flicker : glm::vec3(0.0f));
        shader.setFloat("lights[2].linear", 0.14f);
        shader.setFloat("lights[2].quadratic", 0.07f);

        // ---- Chão, teto e paredes ----
        drawObj(shader, plane, makeModel({0, 0, 0}, {10, 1, 10}), Mat::wood(), texFloor, {5, 5});
        drawObj(shader, cube, makeModel({0, 3.1f, 0}, {10.4f, 0.2f, 10.4f}), Mat::wall(), texWall, {3, 3});   // teto
        drawObj(shader, cube, makeModel({0, 1.5f, -5}, {10, 3, 0.2f}), Mat::wall(), texWall, {3, 1});        // fundo
        drawObj(shader, cube, makeModel({0, 1.5f, 5}, {10, 3, 0.2f}), Mat::wall(), texWall, {3, 1});         // frente
        drawObj(shader, cube, makeModel({-5, 1.5f, 0}, {0.2f, 3, 10}), Mat::wall(), texWall, {3, 1});        // esquerda
        // parede direita em 3 pedaços, deixando um vão de porta (z de 1 a 2, altura 2.2)
        drawObj(shader, cube, makeModel({5, 1.5f, -2.0f}, {0.2f, 3, 6}), Mat::wall(), texWall, {2, 1});
        drawObj(shader, cube, makeModel({5, 1.5f, 3.5f}, {0.2f, 3, 3}), Mat::wall(), texWall, {1, 1});
        drawObj(shader, cube, makeModel({5, 2.6f, 1.5f}, {0.2f, 0.8f, 1}), Mat::wall(), texWall);

        // ---- Porta: HIERARQUIA (a dobradiça é a origem local) ----
        glm::mat4 doorBase(1.0f);
        doorBase = glm::translate(doorBase, glm::vec3(5.0f, 0.0f, 1.0f));              // 1) vai até a dobradiça
        doorBase = glm::rotate(doorBase, glm::radians(doorAngle), glm::vec3(0, 1, 0)); // 2) gira em torno dela
        glm::mat4 panel = glm::translate(doorBase, glm::vec3(0.0f, 1.1f, 0.5f));       // 3) painel começa na dobradiça
        panel = glm::scale(panel, glm::vec3(0.08f, 2.2f, 1.0f));
        drawObj(shader, cube, panel, Mat::wood(), texWood);
        glm::mat4 handle = glm::translate(doorBase, glm::vec3(-0.07f, 1.0f, 0.88f));    // maçaneta: filha da porta
        handle = glm::scale(handle, glm::vec3(0.08f));
        drawObj(shader, sphere, handle, Mat::metal(), white);

        // ---- Mesa: tampo + 4 pernas ----
        drawObj(shader, cube, makeModel({0, 0.75f, 0}, {1.6f, 0.1f, 0.9f}), Mat::wood(), texWood);
        float lx[] = {-0.7f, 0.7f}, lz[] = {-0.35f, 0.35f};
        for (float x : lx) for (float z : lz)
            drawObj(shader, cube, makeModel({x, 0.35f, z}, {0.1f, 0.7f, 0.1f}), Mat::wood(), texWood);

        // ---- Abajur ----
        drawObj(shader, cylinder, makeModel({0, 0.825f, 0}, {0.25f, 0.05f, 0.25f}), Mat::metal(), white);
        drawObj(shader, cylinder, makeModel({0, 1.05f, 0}, {0.04f, 0.4f, 0.04f}), Mat::metal(), white);
        glm::vec3 glow = lampOn ? glm::vec3(1.0f, 0.9f, 0.6f) : glm::vec3(0.0f);
        drawObj(shader, sphere, makeModel({0, 1.3f, 0}, {0.2f, 0.2f, 0.2f}), Mat::glowing(glow), white);

        // ---- Esfera de metal girando ----
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.95f, 0.0f));
        m = glm::rotate(m, now, glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(0.3f));
        drawObj(shader, sphere, m, Mat::metal(), white);

        // ---- TV: móvel + moldura + tela ----
        drawObj(shader, cube, makeModel({2.5f, 0.3f, -4.6f}, {1.6f, 0.6f, 0.5f}), Mat::wood(), texWood);
        drawObj(shader, cube, makeModel({2.5f, 1.2f, -4.85f}, {1.4f, 0.8f, 0.08f}), tvFrame, white);
        glm::vec3 screen = glm::vec3(0.2f, 0.5f, 0.95f) * flicker;
        drawObj(shader, cube, makeModel({2.5f, 1.2f, -4.80f}, {1.3f, 0.7f, 0.02f}),
                tvOn ? Mat::glowing(screen) : tvOff, white);

        // ---- Ventilador de teto: haste + motor + 4 pás (hierarquia) ----
        glm::vec3 fanPos(-2.5f, 2.75f, -2.5f);
        drawObj(shader, cylinder, makeModel({-2.5f, 2.9f, -2.5f}, {0.04f, 0.2f, 0.04f}), Mat::metal(), white);
        drawObj(shader, cylinder, makeModel(fanPos, {0.25f, 0.12f, 0.25f}), Mat::metal(), white);
        glm::mat4 fanBase = glm::translate(glm::mat4(1.0f), fanPos);
        fanBase = glm::rotate(fanBase, glm::radians(fanAngle), glm::vec3(0, 1, 0));
        for (int i = 0; i < 4; i++) {
            glm::mat4 blade = glm::rotate(fanBase, glm::radians(i * 90.0f), glm::vec3(0, 1, 0));
            blade = glm::translate(blade, glm::vec3(0.5f, 0.0f, 0.0f));
            blade = glm::scale(blade, glm::vec3(0.8f, 0.02f, 0.15f));
            drawObj(shader, cube, blade, Mat::wood(), texWood);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}