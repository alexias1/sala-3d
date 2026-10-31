#include <glad/glad.h>
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
#include "Hud.h"

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

// ---------- Estado da MISSAO ----------
bool candleLit[3] = {false, false, false};
bool candyGot[3]  = {false, false, false};
bool potionDone = false;   // doces jogados no caldeirao
bool keyGot = false;       // pegou a chave (destranca a porta)
bool escaped = false;      // abriu a porta com a chave: fim
double escapeTime = 0.0;
std::string toast;         // mensagem temporaria na tela
double toastUntil = 0.0;
void say(const std::string& m, double secs = 3.5) { toast = m; toastUntil = glfwGetTime() + secs; }

// Posicao (x,z) das velas sobre a mesa
const glm::vec2 CANDLE_XZ[3] = {{-0.7f, -0.28f}, {0.65f, 0.3f}, {0.2f, -0.3f}};
// Doces escondidos pela sala
const glm::vec3 CANDY_POS[3] = {{-3.9f, 0.15f, -3.2f}, {3.9f, 0.15f, 3.9f}, {-1.2f, 0.15f, -3.8f}};
const glm::vec3 CAULDRON(3.3f, 0.0f, -2.0f);
const glm::vec3 KEY_POS = CAULDRON + glm::vec3(0.0f, 1.35f, 0.0f);

// Posicao da abobora grande (tambem e uma fonte de luz)
const glm::vec3 BIG_PUMPKIN(-3.2f, 0.0f, 2.2f);

// Cada item interativo tem um ID; a ordem aqui = ordem do array items[]
enum ItemId { I_DOOR, I_TV, I_FAN, I_LAMP, I_PUMPKIN,
              I_CANDLE0, I_CANDLE1, I_CANDLE2,
              I_CANDY0, I_CANDY1, I_CANDY2,
              I_CAULDRON, I_KEY, I_COUNT };

struct Interactable { const char* name; glm::vec3 pos; };
Interactable items[I_COUNT] = {
    {"porta",      {5.0f, 1.0f, 1.5f}},
    {"TV",         {2.5f, 1.2f, -4.8f}},
    {"ventilador", {-2.5f, 2.8f, -2.5f}},
    {"abajur",     {0.0f, 1.3f, 0.0f}},
    {"abobora",    {-3.2f, 0.4f, 2.2f}},
    {"vela",       {CANDLE_XZ[0].x, 0.95f, CANDLE_XZ[0].y}},
    {"vela",       {CANDLE_XZ[1].x, 0.95f, CANDLE_XZ[1].y}},
    {"vela",       {CANDLE_XZ[2].x, 0.95f, CANDLE_XZ[2].y}},
    {"doce",       CANDY_POS[0]},
    {"doce",       CANDY_POS[1]},
    {"doce",       CANDY_POS[2]},
    {"caldeirao",  {CAULDRON.x, 0.6f, CAULDRON.z}},
    {"chave",      KEY_POS}
};
const float REACH = 3.0f;   // alcance (metros)

int candlesLit() { return (int)candleLit[0] + (int)candleLit[1] + (int)candleLit[2]; }
int candyCount() { return (int)candyGot[0] + (int)candyGot[1] + (int)candyGot[2]; }

// Item aparece/pode ser usado agora?
bool itemActive(int id) {
    if (id >= I_CANDLE0 && id <= I_CANDLE2) return !candleLit[id - I_CANDLE0];
    if (id >= I_CANDY0 && id <= I_CANDY2)   return !candyGot[id - I_CANDY0];
    if (id == I_CAULDRON) return !potionDone;
    if (id == I_KEY)      return potionDone && !keyGot;
    return true;
}

// Item para o qual a camera esta APONTANDO (angulo ~21 graus) e ao alcance.
// Tecnica: produto escalar entre a direcao ao item e o vetor "front" da camera.
int aimedItem() {
    int best = -1; float bestDot = 0.93f;
    for (int i = 0; i < I_COUNT; i++) {
        if (!itemActive(i)) continue;
        glm::vec3 d = items[i].pos - camera.position;
        float dist = glm::length(d);
        if (dist > REACH || dist < 1e-3f) continue;
        float dp = glm::dot(d / dist, camera.front);
        if (dp > bestDot) { bestDot = dp; best = i; }
    }
    return best;
}

// Texto da dica "[E] ..." para o item apontado
std::string promptFor(int id) {
    switch (id) {
        case I_DOOR:    return keyGot ? (doorTarget ? "[E] fechar a porta" : "[E] abrir a porta") : "[E] porta (trancada)";
        case I_TV:      return tvOn ? "[E] desligar a TV" : "[E] ligar a TV";
        case I_FAN:     return fanOn ? "[E] desligar o ventilador" : "[E] ligar o ventilador";
        case I_LAMP:    return lampOn ? "[E] apagar o abajur" : "[E] acender o abajur";
        case I_PUMPKIN: return pumpkinOn ? "[E] apagar as aboboras" : "[E] acender as aboboras";
        case I_CANDLE0: case I_CANDLE1: case I_CANDLE2: return "[E] acender a vela";
        case I_CANDY0: case I_CANDY1: case I_CANDY2:    return "[E] pegar o doce";
        case I_CAULDRON: return "[E] examinar o caldeirao";
        case I_KEY:     return "[E] pegar a chave";
    }
    return "";
}

// Tudo que acontece ao apertar E em um item
void interact(int id) {
    switch (id) {
    case I_DOOR:
        if (!keyGot) { say("A porta esta trancada... ache a chave!"); break; }
        doorTarget = !doorTarget;
        if (doorTarget && !escaped) { escaped = true; escapeTime = glfwGetTime(); }
        break;
    case I_TV:      tvOn = !tvOn; break;
    case I_FAN:     fanOn = !fanOn; break;
    case I_LAMP:    lampOn = !lampOn; break;
    case I_PUMPKIN: pumpkinOn = !pumpkinOn; break;
    case I_CANDLE0: case I_CANDLE1: case I_CANDLE2:
        candleLit[id - I_CANDLE0] = true;
        if (candlesLit() == 3) say("As velas formaram o circulo! Agora ache os doces perdidos.");
        else say("Vela acesa (" + std::to_string(candlesLit()) + "/3)", 2.0);
        break;
    case I_CANDY0: case I_CANDY1: case I_CANDY2:
        candyGot[id - I_CANDY0] = true;
        say("Doce coletado (" + std::to_string(candyCount()) + "/3)", 2.0);
        if (candyCount() == 3) say("Tem os 3 doces! Leve ao caldeirao.");
        break;
    case I_CAULDRON:
        if (candlesLit() < 3)      say("O caldeirao esta frio... acenda as 3 velas da mesa.");
        else if (candyCount() < 3) say("O caldeirao pede 3 doces (voce tem " + std::to_string(candyCount()) + ").");
        else { potionDone = true; say("O caldeirao aceitou a oferenda! Uma chave surgiu..."); }
        break;
    case I_KEY:
        keyGot = true;
        say("Voce pegou a chave! A porta foi destrancada.");
        break;
    }
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
        int i = aimedItem();
        if (i >= 0) interact(i);
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
             unsigned int tex, glm::vec2 uvScale = glm::vec2(1.0f), unsigned int nmap = 0) {
    s.setMat4("model", model);
    s.setVec3("material.ka", m.ka);
    s.setVec3("material.kd", m.kd);
    s.setVec3("material.ks", m.ks);
    s.setFloat("material.shininess", m.shininess);
    s.setVec3("material.emissive", m.emissive);
    s.setVec2("uvScale", uvScale);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    s.setFloat("normalStrength", nmap ? 2.0f : 0.0f);   // 2.0 = relevo mais forte; 0 = sem relevo
    if (nmap) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, nmap);
        glActiveTexture(GL_TEXTURE0);
    }
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
    unsigned int nrmFloor = loadNormalMap("assets/textures/floor_normal.jpg");
    unsigned int nrmWall  = loadNormalMap("assets/textures/wall_normal.jpg");

    // Materiais extras da TV
    Material tvFrame{{0.05f,0.05f,0.05f}, {0.1f,0.1f,0.1f}, {0.3f,0.3f,0.3f}, 32.0f};
    Material tvOff  {{0.02f,0.02f,0.02f}, {0.03f,0.03f,0.03f}, {0.8f,0.8f,0.8f}, 96.0f};

    float escapeFade = 0.0f;
    Hud hud;
    double startTime = glfwGetTime();

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

        int aimed = aimedItem();   // item que o jogador está olhando (usado no HUD)

        // fade do ambiente quando escapa (neblina clareia, amanhece)
        escapeFade = approach(escapeFade, (escaped && doorTarget) ? 1.0f : 0.0f, deltaTime * 0.25f);
        glm::vec3 fogCol = glm::mix(FOG_COLOR, glm::vec3(0.45f, 0.30f, 0.40f), escapeFade);
        float fogDen = glm::mix(FOG_DENSITY, 0.08f, escapeFade);

        // ---------- Desenho ----------
        glClearColor(fogCol.r, fogCol.g, fogCol.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("fogColor", fogCol);
        shader.setFloat("fogDensity", fogDen);
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        shader.setMat4("projection", glm::perspective(glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
        shader.setMat4("view", camera.getViewMatrix());
        shader.setVec3("viewPos", camera.position);
        shader.setInt("tex", 0);
        shader.setInt("normalMap", 1);

        // ---- Luzes (o shader aceita até 4) ----
        float flicker  = 0.85f + 0.15f * std::sin(now * 9.0f);                           // TV "pisca"
        float flickerP = 0.80f + 0.20f * std::sin(now * 7.0f + 1.0f) * std::sin(now * 3.1f); // chama da abóbora
        shader.setInt("numLights", 8);
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
        // luz do caldeirao: verde; roxa depois do ritual
        shader.setVec3("lights[4].position", CAULDRON + glm::vec3(0.0f, 0.9f, 0.0f));
        shader.setVec3("lights[4].color", (potionDone ? glm::vec3(0.7f, 0.25f, 1.0f) : glm::vec3(0.2f, 0.9f, 0.15f)) * (0.9f + 0.1f * std::sin(now * 3.0f)));
        shader.setFloat("lights[4].linear", 0.22f);
        shader.setFloat("lights[4].quadratic", 0.18f);
        // uma luz pequena por vela (so quando acesa)
        for (int i = 0; i < 3; i++) {
            std::string L = "lights[" + std::to_string(5 + i) + "]";
            float fl = 0.8f + 0.2f * std::sin(now * (8.0f + i * 1.7f) + i * 2.0f);
            shader.setVec3(L + ".position", glm::vec3(CANDLE_XZ[i].x, 1.05f, CANDLE_XZ[i].y));
            shader.setVec3(L + ".color", candleLit[i] ? glm::vec3(1.0f, 0.6f, 0.2f) * 0.8f * fl : glm::vec3(0.0f));
            shader.setFloat(L + ".linear", 0.7f);
            shader.setFloat(L + ".quadratic", 1.8f);
        }

        // ---- Chão, teto e paredes ----
        drawObj(shader, plane, makeModel({0, 0, 0}, {10, 1, 10}), Mat::floorDark(), texFloor, {5, 5}, nrmFloor);
        drawObj(shader, cube, makeModel({0, 3.1f, 0}, {10.4f, 0.2f, 10.4f}), Mat::wallHalloween(), texWall, {3, 3}, nrmWall);   // teto
        drawObj(shader, cube, makeModel({0, 1.5f, -5}, {10, 3, 0.2f}), Mat::wallHalloween(), texWall, {3, 1}, nrmWall);        // fundo
        drawObj(shader, cube, makeModel({0, 1.5f, 5}, {10, 3, 0.2f}), Mat::wallHalloween(), texWall, {3, 1}, nrmWall);         // frente
        drawObj(shader, cube, makeModel({-5, 1.5f, 0}, {0.2f, 3, 10}), Mat::wallHalloween(), texWall, {3, 1}, nrmWall);        // esquerda
        // parede direita em 3 pedaços, deixando um vão de porta (z de 1 a 2, altura 2.2)
        drawObj(shader, cube, makeModel({5, 1.5f, -2.0f}, {0.2f, 3, 6}), Mat::wallHalloween(), texWall, {2, 1}, nrmWall);
        drawObj(shader, cube, makeModel({5, 1.5f, 3.5f}, {0.2f, 3, 3}), Mat::wallHalloween(), texWall, {1, 1}, nrmWall);
        drawObj(shader, cube, makeModel({5, 2.6f, 1.5f}, {0.2f, 0.8f, 1}), Mat::wallHalloween(), texWall, glm::vec2(1.0f), nrmWall);

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
        const glm::vec2* candles = CANDLE_XZ;
        for (int i = 0; i < 3; i++) {
            float fl = 0.8f + 0.2f * std::sin(now * (8.0f + i * 1.7f) + i * 2.0f);
            drawObj(shader, cylinder, makeModel({candles[i].x, 0.87f, candles[i].y}, {0.05f, 0.14f, 0.05f}), Mat::wax(), white);
            if (candleLit[i])   // so tem chama depois que o jogador acende
                drawObj(shader, sphere, makeModel({candles[i].x, 0.98f, candles[i].y}, {0.035f, 0.07f * fl + 0.03f, 0.035f}),
                        Mat::glowing(glm::vec3(1.0f, 0.7f, 0.2f) * fl), white);
            else   // pavio apagado
                drawObj(shader, cylinder, makeModel({candles[i].x, 0.96f, candles[i].y}, {0.008f, 0.04f, 0.008f}), Mat::black(), white);
        }

        // ---- Caldeirão borbulhante ----
        {
            glm::vec3 cp(3.3f, 0.0f, -2.0f);
            drawObj(shader, sphere,   makeModel(cp + glm::vec3(0, 0.30f, 0), {0.85f, 0.60f, 0.85f}), Mat::black(), white);
            drawObj(shader, cylinder, makeModel(cp + glm::vec3(0, 0.55f, 0), {0.78f, 0.20f, 0.78f}), Mat::black(), white);
            drawObj(shader, cylinder, makeModel(cp + glm::vec3(0, 0.66f, 0), {0.62f, 0.02f, 0.62f}),
                    Mat::glowing((potionDone ? glm::vec3(0.7f, 0.25f, 1.0f) : glm::vec3(0.2f, 0.9f, 0.15f)) * (0.8f + 0.2f * std::sin(now * 3.0f))), white);
            // bolhas subindo
            for (int i = 0; i < 4; i++) {
                float t = std::fmod(now * 0.45f + i * 0.25f, 1.0f);
                float a = i * 1.9f;
                glm::vec3 bp = cp + glm::vec3(0.12f * std::cos(a), 0.68f + t * 0.45f, 0.12f * std::sin(a));
                float sz = 0.07f * (1.0f - t * 0.6f);
                drawObj(shader, sphere, makeModel(bp, glm::vec3(sz)), Mat::glowing(potionDone ? glm::vec3(0.8f, 0.4f, 1.0f) : glm::vec3(0.3f, 1.0f, 0.2f)), white);
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
            Material gm = Mat::glowing(potionDone ? glm::vec3(0.55f, 0.75f, 0.55f) : glm::vec3(0.42f, 0.48f, 0.62f));   // fica "em paz" depois do ritual
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

        // =================================================================
        //                    OBJETOS DA MISSAO
        // =================================================================
        // Doces: giram e sobem/descem levemente; somem quando coletados
        for (int i = 0; i < 3; i++) {
            if (candyGot[i]) continue;
            glm::vec3 cp = CANDY_POS[i] + glm::vec3(0, 0.05f * std::sin(now * 2.0f + i), 0);
            glm::mat4 cb = glm::rotate(glm::translate(glm::mat4(1.0f), cp), now * 1.5f + i, glm::vec3(0, 1, 0));
            Material body = Mat::glowing(glm::vec3(1.0f, 0.45f, 0.05f) * 0.8f);
            drawObj(shader, sphere, glm::scale(cb, {0.16f, 0.09f, 0.09f}), body, white);          // bala
            for (int sx = -1; sx <= 1; sx += 2) {                                                   // pontas do papel
                glm::mat4 w = glm::translate(cb, {sx * 0.11f, 0, 0});
                w = glm::rotate(w, glm::radians(45.0f), glm::vec3(1, 0, 0));
                drawObj(shader, cube, glm::scale(w, {0.05f, 0.07f, 0.07f}), Mat::glowing(glm::vec3(0.6f, 0.2f, 0.9f) * 0.8f), white);
            }
        }
        // Chave dourada: aparece sobre o caldeirao depois do ritual
        if (potionDone && !keyGot) {
            glm::vec3 kp = KEY_POS + glm::vec3(0, 0.08f * std::sin(now * 2.5f), 0);
            glm::mat4 kb = glm::rotate(glm::translate(glm::mat4(1.0f), kp), now * 1.2f, glm::vec3(0, 1, 0));
            Material gold{{0.30f, 0.22f, 0.05f}, {0.80f, 0.60f, 0.10f}, {1.0f, 0.9f, 0.4f}, 64.0f, {0.35f, 0.25f, 0.02f}};
            glm::mat4 head = glm::rotate(glm::translate(kb, {0, 0.17f, 0}), glm::radians(90.0f), glm::vec3(1, 0, 0));
            drawObj(shader, cylinder, glm::scale(head, {0.14f, 0.03f, 0.14f}), gold, white);        // argola
            drawObj(shader, cylinder, glm::scale(glm::translate(head, {0, 0, 0}), {0.06f, 0.04f, 0.06f}), Mat::black(), white); // furo
            drawObj(shader, cube, glm::scale(glm::translate(kb, {0, -0.03f, 0}), {0.035f, 0.32f, 0.035f}), gold, white);        // haste
            drawObj(shader, cube, glm::scale(glm::translate(kb, {0.05f, -0.14f, 0}), {0.08f, 0.035f, 0.035f}), gold, white);   // dentes
            drawObj(shader, cube, glm::scale(glm::translate(kb, {0.04f, -0.08f, 0}), {0.06f, 0.035f, 0.035f}), gold, white);
        }

        // =================================================================
        //                              HUD
        // =================================================================
        {
            hud.begin(w, h);
            float cx = w / 2.0f, cy = h / 2.0f;
            const glm::vec3 WHITE(1), GREY(0.6f), GREEN(0.55f, 1.0f, 0.55f), ORANGE(1.0f, 0.65f, 0.15f);

            // painel da missao
            bool done[5] = {candlesLit() == 3, candyCount() == 3, potionDone, keyGot, escaped};
            int cur = 0; while (cur < 5 && done[cur]) cur++;
            std::string lines[5] = {
                std::string(done[0] ? "[x] " : "[ ] ") + "Acender as velas  " + std::to_string(candlesLit()) + "/3",
                std::string(done[1] ? "[x] " : "[ ] ") + "Achar os doces    " + std::to_string(candyCount()) + "/3",
                std::string(done[2] ? "[x] " : "[ ] ") + "Ritual no caldeirao",
                std::string(done[3] ? "[x] " : "[ ] ") + "Pegar a chave",
                std::string(done[4] ? "[x] " : "[ ] ") + "Fugir pela porta"};
            hud.rect(12, 12, 330, 156, glm::vec4(0, 0, 0, 0.55f));
            hud.text(24, 22, "MISSAO: QUEBRE A MALDICAO", 2.0f, ORANGE);
            for (int i = 0; i < 5; i++)
                hud.text(24, 52 + i * 22, lines[i], 2.0f, done[i] ? GREEN : (i == cur ? WHITE : GREY));

            // cronometro
            double t = (escaped ? escapeTime : glfwGetTime()) - startTime;
            char tb[16]; std::snprintf(tb, sizeof(tb), "%02d:%02d", (int)t / 60, (int)t % 60);
            hud.rect(w - 112, 12, 100, 34, glm::vec4(0, 0, 0, 0.55f));
            hud.text(w - 100, 20, tb, 3.0f, WHITE);

            // mira + dica do item apontado
            hud.text(cx - 5, cy - 8, "+", 2.0f, aimed >= 0 ? ORANGE : glm::vec3(1, 1, 1));
            if (aimed >= 0) hud.textCentered(cx, cy + 24, promptFor(aimed), 2.5f, ORANGE);

            // mensagem temporaria
            if (glfwGetTime() < toastUntil) {
                float tw = hud.width(toast, 2.5f);
                hud.rect(cx - tw / 2 - 14, h - 84, tw + 28, 40, glm::vec4(0, 0, 0, 0.6f));
                hud.textCentered(cx, h - 74, toast, 2.5f, WHITE);
            }

            // tela de vitoria
            if (escaped && doorTarget) {
                hud.rect(0, cy - 70, (float)w, 130, glm::vec4(0, 0, 0, 0.6f));
                hud.textCentered(cx, cy - 56, "VOCE ESCAPOU!", 7.0f, ORANGE);
                hud.textCentered(cx, cy + 20, std::string("Feliz Halloween!  Tempo: ") + tb, 3.0f, WHITE);
            }
            hud.end();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}