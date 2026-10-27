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
const float PI_F = 3.14159265f;
const glm::vec3 FOG_COLOR(0.1f, 0.0f, 0.0f);   // cor da neblina
const float FOG_DENSITY = 0.2f;                  // aumente para mais névoa

Camera camera(glm::vec3(0.0f, 1.6f, 3.5f));
float lastX = SCR_W / 2.0f, lastY = SCR_H / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

// ---------- Estado dos objetos interativos ----------
bool lampOn = true, tvOn = false, fanOn = false, doorTarget = false, pumpkinOn = true;
float doorOpen = 0.0f;    // 0 = fechada, 1 = aberta (animado aos poucos)
float fanSpeed = 0.0f;    // graus por segundo (acelera e desacelera)
float fanAngle = 0.0f;

// Posição da abóbora grande (também é uma fonte de luz)
const glm::vec3 BIG_PUMPKIN(-3.2f, 0.0f, 2.2f);

struct Interactable { const char* name; glm::vec3 pos; bool* state; };
Interactable items[] = {
    {"porta",      {5.0f, 1.0f, 1.5f},   &doorTarget},
    {"TV",         {2.5f, 1.2f, -4.8f},  &tvOn},
    {"ventilador", {-2.5f, 2.8f, -2.5f}, &fanOn},
    {"abajur",     {0.0f, 1.3f, 0.0f},   &lampOn},
    {"abobora",    {-3.2f, 0.4f, 2.2f},  &pumpkinOn}
};
const int NITEMS = 5;
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
    glfwWindowHint(GLFW_SAMPLES, 4);   // MSAA 4x: bordas sem serrilhado

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "Sala 3D - Halloween", nullptr, nullptr);
    if (!window) { std::cerr << "Falha ao criar janela\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "Falha ao iniciar GLAD\n"; return -1; }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

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

    // =====================================================================
    //  Helpers de decoração (lambdas dentro do main, enxergam shader/meshes)
    // =====================================================================

    // Linha (cilindro fino) entre dois pontos: usada em teias e pernas de aranha
    auto line = [&](glm::vec3 a, glm::vec3 b, float th, const Material& m) {
        glm::vec3 d = b - a;
        float len = glm::length(d);
        if (len < 1e-4f) return;
        glm::vec3 dir = d / len, up(0, 1, 0);
        glm::vec3 axis = glm::cross(up, dir);
        float s = glm::length(axis);
        glm::mat4 M = glm::translate(glm::mat4(1.0f), (a + b) * 0.5f);
        if (s > 1e-5f) M = glm::rotate(M, std::atan2(s, glm::dot(up, dir)), axis / s);
        else if (dir.y < 0) M = glm::rotate(M, PI_F, glm::vec3(1, 0, 0));
        M = glm::scale(M, glm::vec3(th, len, th));
        drawObj(shader, cylinder, M, m, white);
    };

    // Teia de canto: 5 fios radiais (90 graus) + 3 anéis ligando os fios
    auto web = [&](glm::vec3 corner, glm::vec3 u, glm::vec3 v, float R) {
        const int N = 5;
        glm::vec3 dirs[N];
        for (int k = 0; k < N; k++) {
            float a = k * (PI_F / 2.0f) / (N - 1);
            dirs[k] = u * std::cos(a) + v * std::sin(a);
            line(corner, corner + dirs[k] * R, 0.007f, Mat::web());
        }
        float rings[3] = {0.33f * R, 0.66f * R, R};
        for (float r : rings)
            for (int k = 0; k < N - 1; k++) {
                // leve "barriga" entre fios, como uma teia de verdade
                glm::vec3 p = corner + dirs[k] * r, q = corner + dirs[k + 1] * r;
                glm::vec3 mid = (p + q) * 0.5f - (u + v) * (0.04f * r);
                line(p, mid, 0.005f, Mat::web());
                line(mid, q, 0.005f, Mat::web());
            }
    };

    // Jack-o'-lantern. floorPos = ponto no chão (ou em cima do móvel), s = tamanho,
    // rotY = para onde o rosto aponta, lit = rosto aceso, fl = tremulação da chama
    auto pumpkin = [&](glm::vec3 floorPos, float s, float rotY, bool lit, float fl) {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), floorPos + glm::vec3(0, 0.25f * s, 0));
        b = glm::rotate(b, glm::radians(rotY), glm::vec3(0, 1, 0));
        b = glm::scale(b, glm::vec3(s));
        // corpo achatado + cabinho
        drawObj(shader, sphere, glm::scale(b, glm::vec3(0.6f, 0.5f, 0.6f)), Mat::pumpkin(), white);
        drawObj(shader, cylinder, glm::scale(glm::translate(b, glm::vec3(0, 0.27f, 0)),
                                             glm::vec3(0.06f, 0.12f, 0.06f)), Mat::stem(), white);
        Material face = lit ? Mat::glowing(glm::vec3(1.0f, 0.6f, 0.1f) * fl) : Mat::black();
        // olhos (losangos = cubos girados 45 graus)
        for (int sx = -1; sx <= 1; sx += 2) {
            glm::mat4 e = glm::translate(b, glm::vec3(sx * 0.1f, 0.07f, 0.28f));
            e = glm::rotate(e, glm::radians(45.0f), glm::vec3(0, 0, 1));
            e = glm::scale(e, glm::vec3(0.07f, 0.07f, 0.06f));
            drawObj(shader, cube, e, face, white);
        }
        // boca: 3 blocos em sorriso
        float mx[3] = {-0.13f, 0.0f, 0.13f}, my[3] = {-0.07f, -0.10f, -0.07f};
        for (int i = 0; i < 3; i++) {
            glm::mat4 m = glm::translate(b, glm::vec3(mx[i], my[i], 0.29f));
            m = glm::scale(m, glm::vec3(0.10f, 0.05f, 0.06f));
            drawObj(shader, cube, m, face, white);
        }
    };

    // Lápide: placa + topo arredondado (cilindro deitado)
    auto tomb = [&](glm::vec3 pos, float rotY, float h) {
        drawObj(shader, cube, makeModel(pos + glm::vec3(0, h * 0.5f, 0), {0.5f, h, 0.12f}, rotY), Mat::stone(), white);
        glm::mat4 t = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0, h, 0));
        t = glm::rotate(t, glm::radians(rotY), glm::vec3(0, 1, 0));
        t = glm::rotate(t, glm::radians(90.0f), glm::vec3(1, 0, 0));
        t = glm::scale(t, glm::vec3(0.5f, 0.12f, 0.5f));
        drawObj(shader, cylinder, t, Mat::stone(), white);
        // "RIP": uma cruz gravada em relevo escuro
        glm::mat4 c = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0, h * 0.6f, 0));
        c = glm::rotate(c, glm::radians(rotY), glm::vec3(0, 1, 0));
        drawObj(shader, cube, glm::scale(glm::translate(c, glm::vec3(0, 0, 0.065f)), {0.04f, 0.22f, 0.02f}), Mat::black(), white);
        drawObj(shader, cube, glm::scale(glm::translate(c, glm::vec3(0, 0.05f, 0.065f)), {0.14f, 0.04f, 0.02f}), Mat::black(), white);
    };

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        // ---------- Entrada + colisão ----------
        glm::vec3 old = camera.position;
        processInput(window);
        glm::vec3 np = camera.position;
        // caixa da mesa (com folga de 0.3 m)
        auto blocked = [](float x, float z) {
            if (std::fabs(x) < 1.1f && std::fabs(z) < 0.75f) return true;                  // mesa
            if (std::fabs(x - 3.3f) < 0.7f && std::fabs(z + 2.0f) < 0.7f) return true;     // caldeirão
            if (std::fabs(x - BIG_PUMPKIN.x) < 0.7f && std::fabs(z - BIG_PUMPKIN.z) < 0.7f) return true; // abóbora
            return false;
        };
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
        std::string title = "Sala 3D - Halloween";
        int ni = nearestItem();
        if (ni >= 0)
            title += std::string("   |   [E] ") + items[ni].name +
                     (*items[ni].state ? " (desligar/fechar)" : " (ligar/abrir)");
        if (title != lastTitle) { glfwSetWindowTitle(window, title.c_str()); lastTitle = title; }

        // ---------- Desenho ----------
        glClearColor(FOG_COLOR.r, FOG_COLOR.g, FOG_COLOR.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("fogColor", FOG_COLOR);
        shader.setFloat("fogDensity", FOG_DENSITY);
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        shader.setMat4("projection", glm::perspective(glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
        shader.setMat4("view", camera.getViewMatrix());
        shader.setVec3("viewPos", camera.position);
        shader.setInt("tex", 0);

        // ---- Luzes (o shader aceita até 4) ----
        float flicker  = 0.85f + 0.15f * std::sin(now * 9.0f);                           // TV "pisca"
        float flickerP = 0.80f + 0.20f * std::sin(now * 7.0f + 1.0f) * std::sin(now * 3.1f); // chama da abóbora
        shader.setInt("numLights", 4);
        shader.setVec3("lights[0].position", glm::vec3(0.0f, 1.3f, 0.0f));   // abajur (agora laranja)
        shader.setVec3("lights[0].color", lampOn ? glm::vec3(1.0f, 0.55f, 0.2f) * 1.5f : glm::vec3(0.0f));
        shader.setFloat("lights[0].linear", 0.09f);
        shader.setFloat("lights[0].quadratic", 0.032f);
        shader.setVec3("lights[1].position", glm::vec3(0.0f, 2.8f, 2.0f));  // luar roxo entrando na sala
        shader.setVec3("lights[1].color", glm::vec3(0.30f, 0.14f, 0.45f));
        shader.setFloat("lights[1].linear", 0.09f);
        shader.setFloat("lights[1].quadratic", 0.032f);
        shader.setVec3("lights[2].position", glm::vec3(2.5f, 1.2f, -4.0f)); // TV
        shader.setVec3("lights[2].color", tvOn ? glm::vec3(0.2f, 0.45f, 1.0f) * flicker : glm::vec3(0.0f));
        shader.setFloat("lights[2].linear", 0.14f);
        shader.setFloat("lights[2].quadratic", 0.07f);
        shader.setVec3("lights[3].position", BIG_PUMPKIN + glm::vec3(0.0f, 0.5f, 0.3f)); // abóbora
        shader.setVec3("lights[3].color", pumpkinOn ? glm::vec3(1.0f, 0.45f, 0.08f) * 1.4f * flickerP : glm::vec3(0.0f));
        shader.setFloat("lights[3].linear", 0.22f);
        shader.setFloat("lights[3].quadratic", 0.20f);

        // ---- Chão, teto e paredes ----
        drawObj(shader, plane, makeModel({0, 0, 0}, {10, 1, 10}), Mat::floorDark(), texFloor, {5, 5});
        drawObj(shader, cube, makeModel({0, 3.1f, 0}, {10.4f, 0.2f, 10.4f}), Mat::wallHalloween(), texWall, {3, 3});   // teto
        drawObj(shader, cube, makeModel({0, 1.5f, -5}, {10, 3, 0.2f}), Mat::wallHalloween(), texWall, {3, 1});        // fundo
        drawObj(shader, cube, makeModel({0, 1.5f, 5}, {10, 3, 0.2f}), Mat::wallHalloween(), texWall, {3, 1});         // frente
        drawObj(shader, cube, makeModel({-5, 1.5f, 0}, {0.2f, 3, 10}), Mat::wallHalloween(), texWall, {3, 1});        // esquerda
        // parede direita em 3 pedaços, deixando um vão de porta (z de 1 a 2, altura 2.2)
        drawObj(shader, cube, makeModel({5, 1.5f, -2.0f}, {0.2f, 3, 6}), Mat::wallHalloween(), texWall, {2, 1});
        drawObj(shader, cube, makeModel({5, 1.5f, 3.5f}, {0.2f, 3, 3}), Mat::wallHalloween(), texWall, {1, 1});
        drawObj(shader, cube, makeModel({5, 2.6f, 1.5f}, {0.2f, 0.8f, 1}), Mat::wallHalloween(), texWall);

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

        // ---- Abajur (luz laranja de Halloween) ----
        drawObj(shader, cylinder, makeModel({0, 0.825f, 0}, {0.25f, 0.05f, 0.25f}), Mat::metal(), white);
        drawObj(shader, cylinder, makeModel({0, 1.05f, 0}, {0.04f, 0.4f, 0.04f}), Mat::metal(), white);
        glm::vec3 glow = lampOn ? glm::vec3(1.0f, 0.6f, 0.2f) : glm::vec3(0.0f);
        drawObj(shader, sphere, makeModel({0, 1.3f, 0}, {0.2f, 0.2f, 0.2f}), Mat::glowing(glow), white);

        // ---- Esfera de metal girando ----
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.95f, 0.0f));
        m = glm::rotate(m, now, glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(0.3f));
        drawObj(shader, sphere, m, Mat::glowing(glm::vec3(0.5f)), white);

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
            drawObj(shader, cube, blade, Mat::black(), white);   // pás pretas
        }

        // =================================================================
        //                      DECORAÇÃO DE HALLOWEEN
        // =================================================================

        // ---- Abóboras (todas acendem/apagam junto com a grande, tecla E) ----
        pumpkin(BIG_PUMPKIN,                 1.6f, 55.0f,  pumpkinOn, flickerP);   // grande, perto do canto
        pumpkin({1.9f, 0.6f, -4.55f},        0.6f, 15.0f,  pumpkinOn, flickerP);   // em cima do móvel da TV
        pumpkin({3.1f, 0.6f, -4.55f},        0.5f, -10.0f, pumpkinOn, flickerP * 0.9f);
        pumpkin({4.3f, 0.0f, 0.2f},          0.9f, -70.0f, pumpkinOn, flickerP);   // perto da porta
        pumpkin({-4.2f, 0.0f, -4.2f},        1.0f, 40.0f,  pumpkinOn, flickerP);   // canto do fundo

        // ---- Caveira em cima da mesa ----
        {
            glm::mat4 sb = glm::translate(glm::mat4(1.0f), glm::vec3(-0.45f, 0.8f, 0.2f));
            sb = glm::rotate(sb, glm::radians(25.0f), glm::vec3(0, 1, 0));
            drawObj(shader, sphere, glm::scale(glm::translate(sb, {0, 0.07f, 0}), {0.14f, 0.13f, 0.14f}), Mat::bone(), white);
            drawObj(shader, cube,   glm::scale(glm::translate(sb, {0, 0.02f, 0.025f}), {0.08f, 0.04f, 0.08f}), Mat::bone(), white);
            for (int sx = -1; sx <= 1; sx += 2)
                drawObj(shader, sphere, glm::scale(glm::translate(sb, {sx * 0.03f, 0.08f, 0.055f}), glm::vec3(0.038f)), Mat::black(), white);
            drawObj(shader, cube, glm::scale(glm::translate(sb, {0, 0.05f, 0.068f}), {0.015f, 0.025f, 0.015f}), Mat::black(), white); // nariz
        }

        // ---- Velas na mesa (chama tremulando, cada uma com fase própria) ----
        glm::vec2 candles[3] = {{-0.7f, -0.28f}, {0.65f, 0.3f}, {0.2f, -0.3f}};
        for (int i = 0; i < 3; i++) {
            float fl = 0.8f + 0.2f * std::sin(now * (8.0f + i * 1.7f) + i * 2.0f);
            drawObj(shader, cylinder, makeModel({candles[i].x, 0.87f, candles[i].y}, {0.05f, 0.14f, 0.05f}), Mat::wax(), white);
            drawObj(shader, sphere, makeModel({candles[i].x, 0.98f, candles[i].y}, {0.035f, 0.07f * fl + 0.03f, 0.035f}),
                    Mat::glowing(glm::vec3(1.0f, 0.7f, 0.2f) * fl), white);
        }

        // ---- Caldeirão borbulhante ----
        {
            glm::vec3 cp(3.3f, 0.0f, -2.0f);
            drawObj(shader, sphere,   makeModel(cp + glm::vec3(0, 0.30f, 0), {0.85f, 0.60f, 0.85f}), Mat::black(), white);
            drawObj(shader, cylinder, makeModel(cp + glm::vec3(0, 0.55f, 0), {0.78f, 0.20f, 0.78f}), Mat::black(), white);
            drawObj(shader, cylinder, makeModel(cp + glm::vec3(0, 0.66f, 0), {0.62f, 0.02f, 0.62f}),
                    Mat::glowing(glm::vec3(0.2f, 0.9f, 0.15f) * (0.8f + 0.2f * std::sin(now * 3.0f))), white);
            // bolhas subindo
            for (int i = 0; i < 4; i++) {
                float t = std::fmod(now * 0.45f + i * 0.25f, 1.0f);
                float a = i * 1.9f;
                glm::vec3 bp = cp + glm::vec3(0.12f * std::cos(a), 0.68f + t * 0.45f, 0.12f * std::sin(a));
                float sz = 0.07f * (1.0f - t * 0.6f);
                drawObj(shader, sphere, makeModel(bp, glm::vec3(sz)), Mat::glowing(glm::vec3(0.3f, 1.0f, 0.2f)), white);
            }
            // pezinhos
            for (int i = 0; i < 3; i++) {
                float a = i * 2.0944f + 0.5f;
                drawObj(shader, cylinder, makeModel(cp + glm::vec3(0.28f * std::cos(a), 0.05f, 0.28f * std::sin(a)), {0.07f, 0.12f, 0.07f}), Mat::black(), white);
            }
        }

        // ---- Lápides na parede da esquerda ----
        tomb({-4.55f, 0.0f, -0.6f}, 90.0f, 0.65f);
        tomb({-4.45f, 0.0f,  0.5f}, 80.0f, 0.50f);
        tomb({-4.55f, 0.0f,  1.5f}, 100.0f, 0.75f);

        // ---- Teias de aranha nos cantos do teto ----
        web({-4.89f, 2.98f, -4.89f}, {1, 0, 0},  {0, -1, 0}, 1.3f);   // fundo/esquerda
        web({ 4.89f, 2.98f, -4.89f}, {-1, 0, 0}, {0, -1, 0}, 1.3f);   // fundo/direita
        web({-4.89f, 2.98f,  4.89f}, {0, 0, -1}, {0, -1, 0}, 1.1f);   // parede esquerda, canto da frente
        web({-4.89f, 2.98f, -4.89f}, {0, 0, 1},  {0, -1, 0}, 0.9f);   // parede esquerda, canto do fundo

        // ---- Aranha descendo do teto ----
        {
            glm::vec3 top(1.8f, 2.99f, 1.2f);
            glm::vec3 body = top + glm::vec3(0.0f, -(0.9f + 0.4f * (0.5f + 0.5f * std::sin(now * 0.8f))), 0.0f);
            line(top, body + glm::vec3(0, 0.08f, 0), 0.004f, Mat::web());
            glm::mat4 sb = glm::translate(glm::mat4(1.0f), body);
            sb = glm::rotate(sb, now * 0.6f, glm::vec3(0, 1, 0));   // gira devagar no fio
            drawObj(shader, sphere, glm::scale(sb, {0.14f, 0.11f, 0.18f}), Mat::black(), white);
            drawObj(shader, sphere, glm::scale(glm::translate(sb, {0, 0, 0.13f}), glm::vec3(0.09f)), Mat::black(), white);
            for (int sx = -1; sx <= 1; sx += 2)
                drawObj(shader, sphere, glm::scale(glm::translate(sb, {sx * 0.025f, 0.02f, 0.19f}), glm::vec3(0.02f)),
                        Mat::glowing(glm::vec3(1.0f, 0.0f, 0.0f)), white);
            for (int i = 0; i < 4; i++)
                for (int sx = -1; sx <= 1; sx += 2) {
                    float zz = (i - 1.5f) * 0.07f;
                    glm::vec3 knee = glm::vec3(sb * glm::vec4(sx * 0.17f, 0.10f, zz, 1.0f));
                    glm::vec3 foot = glm::vec3(sb * glm::vec4(sx * 0.30f, -0.12f, zz * 1.6f, 1.0f));
                    glm::vec3 root = glm::vec3(sb * glm::vec4(sx * 0.06f, 0.0f, zz * 0.6f, 1.0f));
                    line(root, knee, 0.012f, Mat::black());
                    line(knee, foot, 0.010f, Mat::black());
                }
        }

        // ---- Morcegos voando em círculos no teto ----
        for (int i = 0; i < 3; i++) {
            float a = now * (0.8f + 0.2f * i) + i * 2.1f;
            float r = 1.2f + 0.6f * i;
            glm::vec3 bp(std::cos(a) * r, 2.3f + 0.15f * std::sin(now * 3.0f + i), std::sin(a) * r);
            float yaw = std::atan2(-std::cos(a), -std::sin(a));   // nariz (+x local) segue a tangente do círculo
            glm::mat4 bb = glm::translate(glm::mat4(1.0f), bp);
            bb = glm::rotate(bb, yaw, glm::vec3(0, 1, 0));
            drawObj(shader, sphere, glm::scale(bb, {0.14f, 0.09f, 0.09f}), Mat::black(), white);
            float flap = 40.0f * std::sin(now * 14.0f + i);
            for (int sd = -1; sd <= 1; sd += 2) {
                glm::mat4 wg = glm::rotate(bb, glm::radians(sd * flap), glm::vec3(1, 0, 0));
                wg = glm::translate(wg, glm::vec3(0, 0, sd * 0.17f));
                wg = glm::scale(wg, glm::vec3(0.14f, 0.01f, 0.32f));
                drawObj(shader, cube, wg, Mat::black(), white);
            }
        }

        // ---- Fantasma flutuando (sempre olhando para você) ----
        {
            glm::vec3 gp(-3.0f, 1.5f + 0.15f * std::sin(now * 1.5f), -3.6f);
            float yaw = std::atan2(camera.position.x - gp.x, camera.position.z - gp.z);
            glm::mat4 gb = glm::translate(glm::mat4(1.0f), gp);
            gb = glm::rotate(gb, yaw, glm::vec3(0, 1, 0));
            Material gm = Mat::glowing(glm::vec3(0.42f, 0.48f, 0.62f));
            drawObj(shader, sphere,   glm::scale(gb, {0.55f, 0.65f, 0.55f}), gm, white);
            drawObj(shader, cylinder, glm::scale(glm::translate(gb, {0, -0.2f, 0}), {0.55f, 0.45f, 0.55f}), gm, white);
            for (int i = 0; i < 8; i++) {   // barra ondulada da "saia"
                float a = i * 2.0f * PI_F / 8.0f;
                float wv = 0.03f * std::sin(now * 3.0f + i * 1.7f);
                drawObj(shader, sphere, glm::scale(glm::translate(gb, {0.26f * std::cos(a), -0.43f + wv, 0.26f * std::sin(a)}), glm::vec3(0.15f)), gm, white);
            }
            for (int sx = -1; sx <= 1; sx += 2) {
                drawObj(shader, sphere, glm::scale(glm::translate(gb, {sx * 0.09f, 0.08f, 0.24f}), {0.07f, 0.10f, 0.05f}), Mat::black(), white);
                glm::mat4 arm = glm::translate(gb, {sx * 0.30f, -0.05f + 0.05f * std::sin(now * 2.0f + sx), 0.06f});
                arm = glm::rotate(arm, glm::radians(sx * 25.0f), glm::vec3(0, 0, 1));
                drawObj(shader, sphere, glm::scale(arm, {0.10f, 0.22f, 0.10f}), gm, white);
            }
            drawObj(shader, sphere, glm::scale(glm::translate(gb, {0, -0.08f, 0.25f}), {0.08f, 0.12f, 0.04f}), Mat::black(), white);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}