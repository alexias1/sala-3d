#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>
#include <random>
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Hud.h"
const int SCR_W = 1280, SCR_H = 720;
const float PI_F = 3.14159265f;
const glm::vec3 FOG_COLOR(0.1f, 0.0f, 0.0f);
const float FOG_DENSITY = 0.2f;
Camera camera(glm::vec3(0.0f, 1.6f, 3.5f));
float lastX = SCR_W / 2.0f, lastY = SCR_H / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

bool lampOn = true, tvOn = false, fanOn = false, doorTarget = false, pumpkinOn = true;
float doorOpen = 0.0f;
float fanSpeed = 0.0f;
float fanAngle = 0.0f;

// Estado global da missao e das interacoes.
bool candleLit[3] = {false, false, false};
bool candyGot[3] = {false, false, false};
bool potionDone = false;
bool keyGot = false;
bool escaped = false;

bool tombInspected[3] = {false, false, false};
double spiritHintAt = -1.0;

// Tarot: tres cartas reveladas em sequencia, com sorteio ao embaralhar.
int cardStage = 0;
bool cardsShuffling = false;
float cardFlip[3] = {0, 0, 0};
int cardPick[3] = {0, 1, 2};
double cardHintAt = -1.0;

struct TarotCard
{
    const char *name;
    const char *meaning;
};

constexpr int DECK_SIZE = 6;
const TarotCard DECK[DECK_SIZE] = {
    {"A LUA", "segredos vivem no escuro"},
    {"A MORTE", "um fim... e um recomeco"},
    {"A ABOBORA", "doce por fora, susto por dentro"},
    {"O FANTASMA", "alguem nao foi embora"},
    {"A ESTRELA", "uma luz guia seus passos"},
    {"A CHAVE", "o trancado vai se abrir"}};
const char *CARD_SLOT[3] = {"PASSADO", "PRESENTE", "FUTURO"};

void shuffleDeck()
{
    static std::mt19937 rng(std::random_device{}());
    int order[DECK_SIZE];
    for (int i = 0; i < DECK_SIZE; ++i)
        order[i] = i;
    std::shuffle(order, order + DECK_SIZE, rng);
    for (int i = 0; i < 3; ++i)
        cardPick[i] = order[i];
}
bool ghostFound[3] = {false, false, false};

double eyesHiddenUntil = 0.0;
double escapeTime = 0.0;
std::string toast;
double toastUntil = 0.0;
void say(const std::string &m, double secs = 3.5)
{
    toast = m;
    toastUntil = glfwGetTime() + secs;
}

// Posicoes usadas tanto na renderizacao quanto nas interacoes.
const glm::vec2 CANDLE_XZ[3] = {{-0.7f, -0.28f}, {0.65f, 0.3f}, {0.2f, -0.3f}};

const glm::vec3 CANDY_POS[3] = {{-3.9f, 0.15f, -3.2f}, {0.58f, 0.78f, 4.52f}, {-1.2f, 0.15f, -3.8f}};
const glm::vec3 CAULDRON(3.3f, 0.0f, -2.0f);
const glm::vec3 KEY_POS = CAULDRON + glm::vec3(0.0f, 1.35f, 0.0f);

const char *STUDENT_NAMES[3] = {
    "Alexia Rodrigues", "Barbara Albuquerque", "Lara Nogueira"};
const glm::vec3 TOMB_POS[3] = {
    {-4.55f, 0.45f, -0.60f},
    {-4.45f, 0.40f, 0.50f},
    {-4.55f, 0.50f, 1.50f}};
const glm::vec3 GHOST_POS[3] = {
    {-3.0f, 1.50f, -3.60f},
    {-3.0f, 1.50f, 4.0f},
    {3.2f, 1.50f, 3.25f}};

const glm::vec3 GHOST_COLORS[3] = {
    {0.42f, 0.48f, 0.62f},
    {0.51f, 0.39f, 0.65f},
    {0.39f, 0.62f, 0.61f}};

const glm::vec3 BIG_PUMPKIN(-3.2f, 0.0f, 2.2f);

struct PumpkinInfo
{
    glm::vec3 pos;
    float scale;
    float rotY;
    float lightPower;
};
const PumpkinInfo SMALL_PUMPKINS[4] = {
    {{1.9f, 0.6f, -4.55f}, 0.6f, 15.0f, 0.70f},
    {{3.1f, 0.6f, -4.55f}, 0.5f, -10.0f, 0.60f},
    {{4.3f, 0.0f, 0.20f}, 0.9f, -70.0f, 1.00f},
    {{-4.2f, 0.0f, -4.20f}, 1.0f, 40.0f, 1.05f}};

// IDs e ordem de items[] precisam permanecer correspondentes.
enum ItemId
{
    I_DOOR,
    I_TV,
    I_FAN,
    I_LAMP,
    I_PUMPKIN,
    I_CANDLE0,
    I_CANDLE1,
    I_CANDLE2,
    I_CANDY0,
    I_CANDY1,
    I_CANDY2,
    I_CAULDRON,
    I_KEY,
    I_TOMB0,
    I_TOMB1,
    I_TOMB2,
    I_GHOST0,
    I_GHOST1,
    I_GHOST2,
    I_CARDS,
    I_WINDOW,
    I_COUNT
};
struct Interactable
{
    const char *name;
    glm::vec3 pos;
};
Interactable items[I_COUNT] = {
    {"porta", {5.0f, 1.0f, 1.5f}},
    {"TV", {2.5f, 1.2f, -4.8f}},
    {"ventilador", {-2.5f, 2.8f, -2.5f}},
    {"abajur", {0.0f, 1.3f, 0.0f}},
    {"abobora", {-3.2f, 0.4f, 2.2f}},
    {"vela", {CANDLE_XZ[0].x, 0.95f, CANDLE_XZ[0].y}},
    {"vela", {CANDLE_XZ[1].x, 0.95f, CANDLE_XZ[1].y}},
    {"vela", {CANDLE_XZ[2].x, 0.95f, CANDLE_XZ[2].y}},
    {"doce", CANDY_POS[0]},
    {"doce", CANDY_POS[1]},
    {"doce", CANDY_POS[2]},
    {"caldeirao", {CAULDRON.x, 0.6f, CAULDRON.z}},
    {"chave", KEY_POS},
    {"lapide", TOMB_POS[0]},
    {"lapide", TOMB_POS[1]},
    {"lapide", TOMB_POS[2]},
    {"fantasma", GHOST_POS[0]},
    {"fantasma", GHOST_POS[1]},
    {"fantasma", GHOST_POS[2]},
    {"cartas", {0.45f, 0.87f, 0.0f}},
    {"janela", {-1.80f, 1.80f, -4.83f}}};
const float REACH = 3.0f;
int candlesLit() { return (int)candleLit[0] + (int)candleLit[1] + (int)candleLit[2]; }
int candyCount() { return (int)candyGot[0] + (int)candyGot[1] + (int)candyGot[2]; }
int tombCount() { return (int)tombInspected[0] + (int)tombInspected[1] + (int)tombInspected[2]; }
int ghostCount() { return (int)ghostFound[0] + (int)ghostFound[1] + (int)ghostFound[2]; }

std::string destinyHint()
{
    if (candlesLit() < 3)
        return "tres chamas devem arder na mesa";
    if (candyCount() < 3)
        return "tres doces esperam no escuro";
    if (!potionDone)
        return "o caldeirao tem fome...";
    if (!keyGot)
        return "uma chave brilha no caldeirao";
    if (tombCount() < 3)
        return "os mortos tem nomes... leia-os";
    if (ghostCount() < 3)
        return "tres espiritos aguardam voce";
    return "a porta se abrira. Va... se ousar";
}

bool itemActive(int id)
{
    if (id >= I_CANDLE0 && id <= I_CANDLE2)
        return !candleLit[id - I_CANDLE0];
    if (id >= I_CANDY0 && id <= I_CANDY2)
        return !candyGot[id - I_CANDY0];
    if (id == I_CAULDRON)
        return !potionDone;
    if (id == I_KEY)
        return potionDone && !keyGot;
    if (id == I_CARDS)
        return !cardsShuffling;
    return true;
}

// Seleciona o objeto mais alinhado a mira, dentro do alcance.
int aimedItem()
{
    int best = -1;
    float bestDot = 0.93f;
    for (int i = 0; i < I_COUNT; i++)
    {
        if (!itemActive(i))
            continue;
        glm::vec3 d = items[i].pos - camera.position;
        float dist = glm::length(d);
        if (dist > REACH || dist < 1e-3f)
            continue;
        float dp = glm::dot(d / dist, camera.front);
        if (dp > bestDot)
        {
            bestDot = dp;
            best = i;
        }
    }
    return best;
}

std::string promptFor(int id)
{
    switch (id)
    {
    case I_DOOR:
        if (!keyGot)
            return "[E] porta (trancada)";
        if (tombCount() < 3)
            return "[E] porta (examinar lapides)";
        if (ghostCount() < 3)
            return "[E] porta (libertar espiritos)";
        return doorTarget ? "[E] fechar a porta" : "[E] abrir e fugir";
    case I_TV:
        return tvOn ? "[E] desligar a TV" : "[E] ligar a TV";
    case I_FAN:
        return fanOn ? "[E] desligar o ventilador" : "[E] ligar o ventilador";
    case I_LAMP:
        return lampOn ? "[E] apagar o abajur" : "[E] acender o abajur";
    case I_PUMPKIN:
        return pumpkinOn ? "[E] apagar as aboboras" : "[E] acender as aboboras";
    case I_CANDLE0:
    case I_CANDLE1:
    case I_CANDLE2:
        return "[E] acender a vela";
    case I_CANDY0:
    case I_CANDY1:
    case I_CANDY2:
        return "[E] pegar o doce";
    case I_CAULDRON:
        return "[E] examinar o caldeirao";
    case I_KEY:
        return "[E] pegar a chave";
    case I_TOMB0:
    case I_TOMB1:
    case I_TOMB2:
        return tombInspected[id - I_TOMB0] ? "[E] reler lapide" : "[E] examinar lapide";
    case I_GHOST0:
    case I_GHOST1:
    case I_GHOST2:
        return ghostFound[id - I_GHOST0] ? "[E] falar com " + std::string(STUDENT_NAMES[id - I_GHOST0])
                                         : "[E] ouvir o fantasma";
    case I_CARDS:
        if (cardStage == 0)
            return "[E] ver seu futuro";
        if (cardStage == 1)
            return "[E] revelar o presente";
        if (cardStage == 2)
            return "[E] revelar o futuro";
        return "[E] embaralhar as cartas";
    case I_WINDOW:
        return "[E] voce tambem viu?";
    }
    return "";
}

// Eventos disparados com a tecla E.
void interact(int id)
{
    switch (id)
    {
    case I_DOOR:
        if (!keyGot)
        {
            say("A porta esta trancada... ache a chave!");
            break;
        }
        if (tombCount() < 3)
        {
            say("A chave nao basta... examine as tres lapides!");
            break;
        }
        if (ghostCount() < 3)
        {
            say("Os espiritos ainda prendem voce aqui! Encontre os tres.");
            break;
        }
        doorTarget = !doorTarget;
        if (doorTarget && !escaped)
        {
            escaped = true;
            escapeTime = glfwGetTime();
        }
        break;
    case I_TV:
        tvOn = !tvOn;
        break;
    case I_FAN:
        fanOn = !fanOn;
        break;
    case I_LAMP:
        lampOn = !lampOn;
        break;
    case I_PUMPKIN:
        pumpkinOn = !pumpkinOn;
        break;
    case I_CANDLE0:
    case I_CANDLE1:
    case I_CANDLE2:
        candleLit[id - I_CANDLE0] = true;
        if (candlesLit() == 3)
            say("As velas formaram o circulo! Agora ache os doces perdidos.");
        else
            say("Vela acesa (" + std::to_string(candlesLit()) + "/3)", 2.0);
        break;
    case I_CANDY0:
    case I_CANDY1:
    case I_CANDY2:
        candyGot[id - I_CANDY0] = true;
        say("Doce coletado (" + std::to_string(candyCount()) + "/3)", 2.0);
        if (candyCount() == 3)
            say("Tem os 3 doces! Leve ao caldeirao.");
        break;
    case I_CAULDRON:
        if (candlesLit() < 3)
            say("O caldeirao esta frio... acenda as 3 velas da mesa.");
        else if (candyCount() < 3)
            say("O caldeirao pede 3 doces (voce tem " + std::to_string(candyCount()) + ").");
        else
        {
            potionDone = true;
            say("O caldeirao aceitou a oferenda! Uma chave surgiu...");
        }
        break;
    case I_KEY:
        keyGot = true;
        if (ghostCount() == 3)
            say("Voce pegou a chave! A porta foi destrancada.");
        else
            say("Voce pegou a chave! Liberte os espiritos para fugir.");
        break;
    case I_TOMB0:
    case I_TOMB1:
    case I_TOMB2:
    {
        int i = id - I_TOMB0;

        if (tombInspected[i])
        {
            say(std::string("Aqui jaz ") + STUDENT_NAMES[i] + ".", 4.0);
            break;
        }

        tombInspected[i] = true;

        say(std::string("Aqui jaz ") + STUDENT_NAMES[i] + ". (" +
                std::to_string(tombCount()) + "/3)",
            4.5);

        if (tombCount() == 3)
        {
            spiritHintAt = glfwGetTime() + 4.5;
        }

        break;
    }
    case I_GHOST0:
    case I_GHOST1:
    case I_GHOST2:
    {
        int i = id - I_GHOST0;
        if (tombCount() < 3)
        {
            say("LEIA NOSSAS LAPIDES E LIBERTE NOSSOS ESPIRITOS!", 4.5);
            break;
        }
        if (ghostFound[i])
        {
            say(std::string("Espirito libertado: ") + STUDENT_NAMES[i] + ".", 4.0);
            break;
        }
        ghostFound[i] = true;
        if (ghostCount() == 3)
            say("Os tres espiritos foram libertados! A maldicao enfraqueceu.", 5.0);
        else
            say(std::string("Espirito encontrado: ") + STUDENT_NAMES[i] + " (" +
                    std::to_string(ghostCount()) + "/3)",
                4.5);
        break;
    }
    case I_CARDS:
    {
        if (cardsShuffling)
            break;
        if (cardStage < 3)
        {
            const TarotCard &c = DECK[cardPick[cardStage]];
            say(std::string(CARD_SLOT[cardStage]) + ": " + c.name + " - " + c.meaning, 5.0);
            ++cardStage;
            if (cardStage == 3)
                cardHintAt = glfwGetTime() + 5.2;
        }
        else
        {
            cardStage = 0;
            cardsShuffling = true;
            cardHintAt = -1.0;
            say("As cartas se embaralham...", 2.0);
        }
        break;
    }
    case I_WINDOW:

        eyesHiddenUntil = glfwGetTime() + 6.0;
        say("Voce tambem viu? Algo nos observava entre as lapides...", 5.0);
        break;
    }
}
void framebuffer_size_callback(GLFWwindow *, int w, int h) { glViewport(0, 0, w, h); }
void mouse_callback(GLFWwindow *, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;
    camera.rotate(dx, dy);
}
void key_callback(GLFWwindow *, int key, int, int action, int)
{
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_E)
    {
        int i = aimedItem();
        if (i >= 0)
            interact(i);
    }
}
void processInput(GLFWwindow *window)
{
    auto down = [&](int k1, int k2)
    {
        return glfwGetKey(window, k1) == GLFW_PRESS || glfwGetKey(window, k2) == GLFW_PRESS;
    };
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (down(GLFW_KEY_W, GLFW_KEY_UP))
        camera.moveForward(deltaTime);
    if (down(GLFW_KEY_S, GLFW_KEY_DOWN))
        camera.moveBackward(deltaTime);
    if (down(GLFW_KEY_A, GLFW_KEY_LEFT))
        camera.moveLeft(deltaTime);
    if (down(GLFW_KEY_D, GLFW_KEY_RIGHT))
        camera.moveRight(deltaTime);
}
// Envia as propriedades do material ao shader e desenha a malha.
void drawObj(Shader &s, const Mesh &mesh, const glm::mat4 &model, const Material &m,
             unsigned int tex, glm::vec2 uvScale = glm::vec2(1.0f), unsigned int nmap = 0)
{
    s.setMat4("model", model);
    s.setVec3("material.ka", m.ka);
    s.setVec3("material.kd", m.kd);
    s.setVec3("material.ks", m.ks);
    s.setFloat("material.shininess", m.shininess);
    s.setVec3("material.emissive", m.emissive);
    s.setVec2("uvScale", uvScale);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    s.setFloat("normalStrength", nmap ? 2.0f : 0.0f);
    if (nmap)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, nmap);
        glActiveTexture(GL_TEXTURE0);
    }
    mesh.draw();
}
glm::mat4 makeModel(glm::vec3 pos, glm::vec3 scale, float rotY = 0.0f)
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(rotY), glm::vec3(0, 1, 0));
    m = glm::scale(m, scale);
    return m;
}

float approach(float v, float alvo, float vel)
{
    if (v < alvo)
        return std::min(alvo, v + vel);
    return std::max(alvo, v - vel);
}
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow *window = glfwCreateWindow(SCR_W, SCR_H, "Sala 3D - Halloween", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar janela\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao iniciar GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    Shader shader("shaders/phong.vert", "shaders/phong.frag");
    Shader crystalGlowShader(
        "shaders/crystal_glow.vert",
        "shaders/crystal_glow.frag");

    Shader groundFogShader("shaders/phong.vert", "shaders/ground_fog.frag");
    Shader moonBeamShader("shaders/phong.vert", "shaders/moon_beam.frag");

    Shader graveyardShader("shaders/phong.vert", "shaders/graveyard.frag");

    Shader eyesGlowShader("shaders/phong.vert", "shaders/eyes_glow.frag");
    Mesh cube = Mesh::cube();
    Mesh plane = Mesh::plane();
    Mesh cylinder = Mesh::cylinder();
    Mesh sphere = Mesh::sphere();

    Mesh skySphere = Mesh::sphere(48, 96);

    // Relevo circular distante: evita bordas visiveis pela janela.
    constexpr int MOUNTAIN_SEGMENTS = 192;
    std::vector<Vertex> mountainVerts;
    std::vector<unsigned int> mountainIdx;
    mountainVerts.reserve((MOUNTAIN_SEGMENTS + 1) * 2);
    mountainIdx.reserve(MOUNTAIN_SEGMENTS * 6);

    for (int i = 0; i <= MOUNTAIN_SEGMENTS; ++i)
    {
        const float a = 2.0f * PI_F * (float)i / (float)MOUNTAIN_SEGMENTS;

        const float radius = 43.0f + 1.6f * std::sin(5.0f * a + 0.7f) + 0.9f * std::cos(13.0f * a + 0.2f);

        const float height = 2.0f + 2.1f * std::pow(std::fabs(std::sin(7.0f * a + 0.4f)), 4.0f) + 1.1f * std::pow(std::fabs(std::sin(17.0f * a + 1.2f)), 5.0f) + 0.35f * std::sin(4.0f * a + 0.3f);

        const float x = radius * std::cos(a);
        const float z = radius * std::sin(a);
        const glm::vec3 normal(-std::cos(a), 0.0f, -std::sin(a));

        mountainVerts.push_back({{x, -5.0f, z}, normal, {0.0f, 0.0f}});
        mountainVerts.push_back({{x, height, z}, normal, {0.0f, 1.0f}});

        if (i > 0)
        {
            const unsigned int prev = (unsigned int)(i - 1) * 2;
            const unsigned int curr = (unsigned int)i * 2;

            mountainIdx.insert(mountainIdx.end(), {prev, curr, curr + 1,
                                                   prev, curr + 1, prev + 1});
        }
    }

    Mesh mountains(mountainVerts, mountainIdx);

    // Geometria translucida dos feixes de luar que cruzam a janela.
    std::vector<Vertex> moonBeamVertices;
    std::vector<unsigned int> moonBeamIndices;

    auto addMoonBeamQuad = [&](glm::vec3 a, glm::vec3 b,
                               glm::vec3 c, glm::vec3 d)
    {
        const unsigned int first = (unsigned int)moonBeamVertices.size();
        const glm::vec3 n(0.0f, 0.0f, 1.0f);
        moonBeamVertices.push_back({a, n, {0.0f, 0.0f}});
        moonBeamVertices.push_back({b, n, {1.0f, 0.0f}});
        moonBeamVertices.push_back({c, n, {1.0f, 1.0f}});
        moonBeamVertices.push_back({d, n, {0.0f, 1.0f}});
        moonBeamIndices.insert(moonBeamIndices.end(),
                               {first, first + 1, first + 2,
                                first, first + 2, first + 3});
    };

    const float beamNearZ = -4.77f;
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 2; ++col)
        {

            const float x0 = -1.8f + (col == 0 ? -0.55f : 0.55f);
            const float y0 = 1.8f + (row == 0 ? -0.35f : 0.35f);

            const float reach = (row == 0 ? 4.2f : 6.0f);
            const float farZ = beamNearZ + reach;
            const float farY = y0 - 0.30f * reach;
            const float farX = x0 - 0.014f * reach;

            for (int band = 0; band < 3; ++band)
            {
                float offset = (float)(band - 1) * 0.13f;
                addMoonBeamQuad(
                    {x0 - 0.42f, y0 + offset, beamNearZ},
                    {x0 + 0.42f, y0 + offset, beamNearZ},
                    {farX + 0.68f, farY + offset * 0.6f, farZ},
                    {farX - 0.68f, farY + offset * 0.6f, farZ});
            }

            for (int band = 0; band < 2; ++band)
            {
                float offset = (band == 0 ? -0.16f : 0.16f);
                addMoonBeamQuad(
                    {x0 + offset, y0 - 0.25f, beamNearZ},
                    {x0 + offset, y0 + 0.25f, beamNearZ},
                    {farX + offset * 1.3f, farY + 0.26f, farZ},
                    {farX + offset * 1.3f, farY - 0.26f, farZ});
            }
        }
    }
    Mesh moonBeams(moonBeamVertices, moonBeamIndices);

    unsigned int white = makeWhiteTexture();
    unsigned int texWood = loadTexture("assets/textures/wood.jpg");
    unsigned int texFloor = loadTexture("assets/textures/floor.jpg");
    unsigned int texWall = loadTexture("assets/textures/wall.jpg");
    unsigned int nrmFloor = loadNormalMap("assets/textures/floor_normal.jpg");
    unsigned int nrmWall = loadNormalMap("assets/textures/wall_normal.jpg");
    unsigned int texMoon = loadTexture("assets/textures/textura_lua.jpg");

    Material tvFrame{{0.05f, 0.05f, 0.05f}, {0.1f, 0.1f, 0.1f}, {0.3f, 0.3f, 0.3f}, 32.0f};
    Material tvOff{{0.02f, 0.02f, 0.02f}, {0.03f, 0.03f, 0.03f}, {0.8f, 0.8f, 0.8f}, 96.0f};
    float escapeFade = 0.0f;
    Hud hud;
    double startTime = glfwGetTime();
    shuffleDeck();

    auto line = [&](glm::vec3 a, glm::vec3 b, float th, const Material &m)
    {
        glm::vec3 d = b - a;
        float len = glm::length(d);
        if (len < 1e-4f)
            return;
        glm::vec3 dir = d / len, up(0, 1, 0);
        glm::vec3 axis = glm::cross(up, dir);
        float s = glm::length(axis);
        glm::mat4 M = glm::translate(glm::mat4(1.0f), (a + b) * 0.5f);
        if (s > 1e-5f)
            M = glm::rotate(M, std::atan2(s, glm::dot(up, dir)), axis / s);
        else if (dir.y < 0)
            M = glm::rotate(M, PI_F, glm::vec3(1, 0, 0));
        M = glm::scale(M, glm::vec3(th, len, th));
        drawObj(shader, cylinder, M, m, white);
    };

    auto web = [&](glm::vec3 corner, glm::vec3 u, glm::vec3 v, float R)
    {
        const int N = 5;
        glm::vec3 dirs[N];
        for (int k = 0; k < N; k++)
        {
            float a = k * (PI_F / 2.0f) / (N - 1);
            dirs[k] = u * std::cos(a) + v * std::sin(a);
            line(corner, corner + dirs[k] * R, 0.007f, Mat::web());
        }
        float rings[3] = {0.33f * R, 0.66f * R, R};
        for (float r : rings)
            for (int k = 0; k < N - 1; k++)
            {

                glm::vec3 p = corner + dirs[k] * r, q = corner + dirs[k + 1] * r;
                glm::vec3 mid = (p + q) * 0.5f - (u + v) * (0.04f * r);
                line(p, mid, 0.005f, Mat::web());
                line(mid, q, 0.005f, Mat::web());
            }
    };

    // Modelo reutilizavel das aboboras, com rosto emissivo quando acesas.
    auto pumpkin = [&](glm::vec3 floorPos, float s, float rotY, bool lit, float fl)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), floorPos + glm::vec3(0, 0.25f * s, 0));
        b = glm::rotate(b, glm::radians(rotY), glm::vec3(0, 1, 0));
        b = glm::scale(b, glm::vec3(s));

        drawObj(shader, sphere, glm::scale(b, glm::vec3(0.6f, 0.5f, 0.6f)), Mat::pumpkin(), white);
        drawObj(shader, cylinder, glm::scale(glm::translate(b, glm::vec3(0, 0.27f, 0)), glm::vec3(0.06f, 0.12f, 0.06f)), Mat::stem(), white);
        Material face = lit ? Mat::glowing(glm::vec3(1.0f, 0.6f, 0.1f) * fl) : Mat::black();

        for (int sx = -1; sx <= 1; sx += 2)
        {
            glm::mat4 e = glm::translate(b, glm::vec3(sx * 0.1f, 0.07f, 0.28f));
            e = glm::rotate(e, glm::radians(45.0f), glm::vec3(0, 0, 1));
            e = glm::scale(e, glm::vec3(0.07f, 0.07f, 0.06f));
            drawObj(shader, cube, e, face, white);
        }

        float mx[3] = {-0.13f, 0.0f, 0.13f}, my[3] = {-0.07f, -0.10f, -0.07f};
        for (int i = 0; i < 3; i++)
        {
            glm::mat4 m = glm::translate(b, glm::vec3(mx[i], my[i], 0.29f));
            m = glm::scale(m, glm::vec3(0.10f, 0.05f, 0.06f));
            drawObj(shader, cube, m, face, white);
        }
    };

    auto tomb = [&](glm::vec3 pos, float rotY, float h)
    {
        drawObj(shader, cube, makeModel(pos + glm::vec3(0, h * 0.5f, 0), {0.5f, h, 0.12f}, rotY), Mat::stone(), white);
        glm::mat4 t = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0, h, 0));
        t = glm::rotate(t, glm::radians(rotY), glm::vec3(0, 1, 0));
        t = glm::rotate(t, glm::radians(90.0f), glm::vec3(1, 0, 0));
        t = glm::scale(t, glm::vec3(0.5f, 0.12f, 0.5f));
        drawObj(shader, cylinder, t, Mat::stone(), white);

        glm::mat4 c = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0, h * 0.6f, 0));
        c = glm::rotate(c, glm::radians(rotY), glm::vec3(0, 1, 0));
        drawObj(shader, cube, glm::scale(glm::translate(c, glm::vec3(0, 0, 0.065f)), {0.04f, 0.22f, 0.02f}), Mat::black(), white);
        drawObj(shader, cube, glm::scale(glm::translate(c, glm::vec3(0, 0.05f, 0.065f)), {0.14f, 0.04f, 0.02f}), Mat::black(), white);
    };
    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();

        if (spiritHintAt >= 0.0 && glfwGetTime() >= spiritHintAt)
        {
            say("Aqui jazem tres alunas... procure seus espiritos!", 5.0);
            spiritHintAt = -1.0;
        }

        if (cardHintAt >= 0.0 && glfwGetTime() >= cardHintAt)
        {
            say(std::string("O destino sussurra: ") + destinyHint(), 6.0);
            cardHintAt = -1.0;
        }
        deltaTime = now - lastFrame;
        lastFrame = now;

        // Colliders no plano XZ; deslocamento dividido em passos para evitar atravessar objetos.
        constexpr float PLAYER_RADIUS = 0.18f;

        auto hitCircle = [&](float x, float z, glm::vec2 center, float radius)
        {
            const float dx = x - center.x;
            const float dz = z - center.y;
            const float limit = radius + PLAYER_RADIUS;
            return dx * dx + dz * dz < limit * limit;
        };

        auto hitBox = [&](float x, float z, glm::vec2 center, glm::vec2 halfSize)
        {
            const float closestX = std::clamp(x, center.x - halfSize.x, center.x + halfSize.x);
            const float closestZ = std::clamp(z, center.y - halfSize.y, center.y + halfSize.y);
            const float dx = x - closestX;
            const float dz = z - closestZ;
            return dx * dx + dz * dz < PLAYER_RADIUS * PLAYER_RADIUS;
        };

        auto blocked = [&](float x, float z)
        {
            if (std::fabs(x) < 1.1f && std::fabs(z) < 0.75f)
                return true;

            if (std::fabs(x - CAULDRON.x) < 0.7f && std::fabs(z - CAULDRON.z) < 0.7f)
                return true;

            if (std::fabs(x - BIG_PUMPKIN.x) < 0.7f && std::fabs(z - BIG_PUMPKIN.z) < 0.7f)
                return true;

            for (const PumpkinInfo &p : SMALL_PUMPKINS)
            {

                const float pumpkinRadius = 0.30f * p.scale;
                if (hitCircle(x, z, {p.pos.x, p.pos.z}, pumpkinRadius))
                    return true;
            }

            if (hitBox(x, z, {2.5f, -4.60f}, {0.80f, 0.25f}))
                return true;

            if (hitBox(x, z, {0.60f, 4.68f}, {0.75f, 0.225f}))
                return true;

            for (int i = 0; i < 3; ++i)
            {
                if (hitBox(x, z, {TOMB_POS[i].x, TOMB_POS[i].z}, {0.14f, 0.30f}))
                    return true;
            }

            for (int i = 0; i < 3; ++i)
            {
                if (hitCircle(x, z, {GHOST_POS[i].x, GHOST_POS[i].z}, 0.35f))
                    return true;
            }

            return false;
        };

        const glm::vec3 old = camera.position;
        processInput(window);
        const glm::vec3 desired = camera.position;

        const glm::vec2 displacement(desired.x - old.x, desired.z - old.z);
        const int steps = std::max(1, (int)std::ceil(glm::length(displacement) / 0.08f));
        const glm::vec2 step = displacement / (float)steps;
        glm::vec2 resolved(old.x, old.z);

        for (int i = 0; i < steps; ++i)
        {

            float nextX = std::clamp(resolved.x + step.x, -4.7f, 4.7f);
            if (!blocked(nextX, resolved.y))
                resolved.x = nextX;

            float nextZ = std::clamp(resolved.y + step.y, -4.7f, 4.7f);
            if (!blocked(resolved.x, nextZ))
                resolved.y = nextZ;
        }

        camera.position = glm::vec3(resolved.x, desired.y, resolved.y);

        doorOpen = approach(doorOpen, doorTarget ? 1.0f : 0.0f, deltaTime * 1.2f);
        float s = doorOpen * doorOpen * (3.0f - 2.0f * doorOpen);
        float doorAngle = -95.0f * s;
        fanSpeed += ((fanOn ? 400.0f : 0.0f) - fanSpeed) * std::min(1.0f, deltaTime * 1.5f);
        fanAngle += fanSpeed * deltaTime;
        int aimed = aimedItem();

        escapeFade = approach(escapeFade, (escaped && doorTarget) ? 1.0f : 0.0f, deltaTime * 0.25f);
        glm::vec3 fogCol = glm::mix(FOG_COLOR, glm::vec3(0.45f, 0.30f, 0.40f), escapeFade);
        float fogDen = glm::mix(FOG_DENSITY, 0.08f, escapeFade);

        glClearColor(fogCol.r, fogCol.g, fogCol.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        shader.setFloat("isMountain", 0.0f);
        shader.setVec3("fogColor", fogCol);
        shader.setFloat("fogDensity", fogDen);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        shader.setMat4("projection", glm::perspective(glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
        shader.setMat4("view", camera.getViewMatrix());
        shader.setVec3("viewPos", camera.position);
        shader.setInt("tex", 0);
        shader.setInt("normalMap", 1);

        // Luz lunar direcional, independente das luzes de ponto.
        const glm::vec3 moonPos(-1.5f, 8.5f, -26.0f);
        const glm::vec3 moonDir = glm::normalize(moonPos - glm::vec3(-1.8f, 1.5f, 0.0f));
        shader.setVec3("moonLightDirection", moonDir);
        shader.setVec3("moonLightColor", glm::vec3(0.36f, 0.48f, 0.72f));
        shader.setFloat("moonIndoorBoost", 4.2f);

        float flicker = 0.85f + 0.15f * std::sin(now * 9.0f);
        float flickerP = 0.80f + 0.20f * std::sin(now * 7.0f + 1.0f) * std::sin(now * 3.1f);
        // Treze luzes de ponto: abajur, TV, aboboras, caldeirao, velas e cristal.
        shader.setInt("numLights", 13);
        shader.setVec3("lights[0].position", glm::vec3(0.0f, 1.3f, 0.0f));
        shader.setVec3("lights[0].color", lampOn ? glm::vec3(1.0f, 0.55f, 0.2f) * 1.5f : glm::vec3(0.0f));
        shader.setFloat("lights[0].linear", 0.09f);
        shader.setFloat("lights[0].quadratic", 0.032f);
        shader.setVec3("lights[1].position", glm::vec3(0.0f, 2.8f, 2.0f));
        shader.setVec3("lights[1].color", glm::vec3(0.30f, 0.14f, 0.45f));
        shader.setFloat("lights[1].linear", 0.09f);
        shader.setFloat("lights[1].quadratic", 0.032f);
        shader.setVec3("lights[2].position", glm::vec3(2.5f, 1.2f, -4.0f));
        shader.setVec3("lights[2].color", tvOn ? glm::vec3(0.2f, 0.45f, 1.0f) * flicker : glm::vec3(0.0f));
        shader.setFloat("lights[2].linear", 0.14f);
        shader.setFloat("lights[2].quadratic", 0.07f);
        shader.setVec3("lights[3].position", BIG_PUMPKIN + glm::vec3(0.0f, 0.5f, 0.3f));
        shader.setVec3("lights[3].color", pumpkinOn ? glm::vec3(1.0f, 0.45f, 0.08f) * 1.4f * flickerP : glm::vec3(0.0f));
        shader.setFloat("lights[3].linear", 0.22f);
        shader.setFloat("lights[3].quadratic", 0.20f);

        shader.setVec3("lights[4].position", CAULDRON + glm::vec3(0.0f, 0.9f, 0.0f));
        shader.setVec3("lights[4].color", (potionDone ? glm::vec3(0.7f, 0.25f, 1.0f) : glm::vec3(0.2f, 0.9f, 0.15f)) * (0.9f + 0.1f * std::sin(now * 3.0f)));
        shader.setFloat("lights[4].linear", 0.22f);
        shader.setFloat("lights[4].quadratic", 0.18f);

        for (int i = 0; i < 3; i++)
        {
            std::string L = "lights[" + std::to_string(5 + i) + "]";
            float fl = 0.8f + 0.2f * std::sin(now * (8.0f + i * 1.7f) + i * 2.0f);
            shader.setVec3(L + ".position", glm::vec3(CANDLE_XZ[i].x, 1.05f, CANDLE_XZ[i].y));
            shader.setVec3(L + ".color", candleLit[i] ? glm::vec3(1.0f, 0.6f, 0.2f) * 0.8f * fl : glm::vec3(0.0f));
            shader.setFloat(L + ".linear", 0.7f);
            shader.setFloat(L + ".quadratic", 1.8f);
        }

        const float crystalLightPulse = 0.84f + 0.16f * std::sin(now * 1.5f);
        shader.setVec3("lights[8].position", glm::vec3(0.83f, 1.455f, 4.47f));
        shader.setVec3("lights[8].color",
                       glm::vec3(0.55f, 0.22f, 0.78f) * crystalLightPulse);
        shader.setFloat("lights[8].linear", 1.5f);
        shader.setFloat("lights[8].quadratic", 8.0f);

        for (int i = 0; i < 4; ++i)
        {
            const PumpkinInfo &p = SMALL_PUMPKINS[i];
            const float a = glm::radians(p.rotY);
            const glm::vec3 frente(std::sin(a), 0.0f, std::cos(a));

            const glm::vec3 lightPos = p.pos + glm::vec3(0.0f, 0.25f * p.scale, 0.0f) + frente * (0.27f * p.scale);

            const float fl = 0.86f + 0.14f * std::sin(now * (6.0f + i * 0.6f) + i * 1.7f);
            const glm::vec3 lightColor = pumpkinOn
                                             ? glm::vec3(1.0f, 0.43f, 0.09f) * (p.lightPower * fl)
                                             : glm::vec3(0.0f);

            const std::string L = "lights[" + std::to_string(9 + i) + "]";
            shader.setVec3(L + ".position", lightPos);
            shader.setVec3(L + ".color", lightColor);
            shader.setFloat(L + ".linear", 0.65f);
            shader.setFloat(L + ".quadratic", 1.10f);
        }

        Material skyMat{
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            1.0f,
            {0.03f, 0.05f, 0.16f}};

        shader.setFloat("fogDensity", 0.0f);
        glDepthMask(GL_FALSE);
        GLboolean skyCullingWasEnabled = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);

        drawObj(shader, skySphere,
                makeModel(camera.position, glm::vec3(160.0f)),
                skyMat, white);

        if (skyCullingWasEnabled)
            glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        shader.setFloat("fogDensity", fogDen);

        drawObj(shader, plane, makeModel({0, 0, 0}, {10, 1, 10}), Mat::floorDark(), texFloor, {5, 5}, nrmFloor);
        drawObj(shader, cube, makeModel({0, 3.1f, 0}, {10.4f, 0.2f, 10.4f}), Mat::wallHalloween(), texWall, {3, 3}, nrmWall);

        float winCenterX = -1.8f;
        float winCenterY = 1.8f;
        float winW = 2.2f;
        float winH = 1.4f;
        float leftEdge = winCenterX - winW * 0.5f;
        float rightEdge = winCenterX + winW * 0.5f;
        float bottomEdge = winCenterY - winH * 0.5f;
        float topEdge = winCenterY + winH * 0.5f;

        drawObj(shader, cube,
                makeModel({(leftEdge - 5.0f) * 0.5f, 1.5f, -5}, {leftEdge + 5.0f, 3.0f, 0.2f}),
                Mat::wallHalloween(), texWall, {2, 1}, nrmWall);

        drawObj(shader, cube,
                makeModel({(rightEdge + 5.0f) * 0.5f, 1.5f, -5}, {5.0f - rightEdge, 3.0f, 0.2f}),
                Mat::wallHalloween(), texWall, {2, 1}, nrmWall);

        drawObj(shader, cube,
                makeModel({winCenterX, bottomEdge * 0.5f, -5}, {winW, bottomEdge, 0.2f}),
                Mat::wallHalloween(), texWall, {1, 1}, nrmWall);

        drawObj(shader, cube,
                makeModel({winCenterX, (topEdge + 3.0f) * 0.5f, -5}, {winW, 3.0f - topEdge, 0.2f}),
                Mat::wallHalloween(), texWall, {1, 1}, nrmWall);

        Material frameMat = Mat::wood();

        drawObj(shader, cube,
                makeModel({leftEdge, winCenterY, -4.93f}, {0.08f, winH + 0.12f, 0.08f}),
                frameMat, texWood);
        drawObj(shader, cube,
                makeModel({rightEdge, winCenterY, -4.93f}, {0.08f, winH + 0.12f, 0.08f}),
                frameMat, texWood);

        drawObj(shader, cube,
                makeModel({winCenterX, topEdge, -4.93f}, {winW + 0.08f, 0.08f, 0.08f}),
                frameMat, texWood);
        drawObj(shader, cube,
                makeModel({winCenterX, bottomEdge, -4.93f}, {winW + 0.08f, 0.08f, 0.08f}),
                frameMat, texWood);

        drawObj(shader, cube,
                makeModel({winCenterX, winCenterY, -4.92f}, {0.05f, winH, 0.04f}),
                frameMat, texWood);
        drawObj(shader, cube,
                makeModel({winCenterX, winCenterY, -4.92f}, {winW, 0.05f, 0.04f}),
                frameMat, texWood);

        {
            // Estante-caixao na parede da frente.
            glm::vec3 shelfCenter = {0.6f, 1.10f, 4.68f};

            const float shelfH = 2.20f;
            const float shelfD = 0.45f;
            const float t = 0.07f;

            const float hb = 0.55f;
            const float hs = 0.75f;
            const float ht = 0.40f;
            const float ys = 0.55f;
            const float top = shelfH / 2;
            const float bot = -shelfH / 2;

            Material shelfMat = Mat::wood();
            shelfMat.ka *= 0.90f;
            shelfMat.kd *= 0.90f;
            shelfMat.ks = glm::vec3(0.015f);

            Material backMat{{0.06f, 0.035f, 0.03f}, {0.17f, 0.10f, 0.08f}, {0.03f, 0.03f, 0.03f}, 6.0f, {0.0f, 0.0f, 0.0f}};

            auto halfW = [&](float y)
            {
                if (y >= ys)
                    return ht + (top - y) / (top - ys) * (hs - ht);
                return hb + (y - bot) / (ys - bot) * (hs - hb);
            };

            const glm::vec2 V[6] = {{-hb, bot}, {hb, bot}, {hs, ys}, {ht, top}, {-ht, top}, {-hs, ys}};

            for (int i = 0; i < 6; i++)
            {
                glm::vec2 A = V[i];
                glm::vec2 B = V[(i + 1) % 6];
                glm::vec2 d = B - A;
                float L = glm::length(d);
                glm::vec2 dir = d / L;
                glm::vec2 n(-dir.y, dir.x);
                glm::vec2 c = (A + B) * 0.5f + n * (t * 0.5f);
                float ang = atan2f(dir.y, dir.x);

                float depth = shelfD - 0.0005f * i;
                float zc = shelfCenter.z + shelfD / 2 - depth / 2;

                glm::mat4 m = glm::translate(glm::mat4(1.0f),
                                             glm::vec3(shelfCenter.x + c.x, shelfCenter.y + c.y, zc));
                m = glm::rotate(m, ang, glm::vec3(0, 0, 1));
                m = glm::scale(m, glm::vec3(L, t, depth));
                drawObj(shader, cube, m, shelfMat, texWood);
            }

            const float step = 0.04f;
            for (float y0 = bot; y0 < top - 0.001f; y0 += step)
            {
                float y1 = std::min(y0 + step, top);
                float w = std::min(halfW(y0), halfW(y1));
                drawObj(shader, cube,
                        makeModel({shelfCenter.x,
                                   shelfCenter.y + (y0 + y1) / 2,
                                   shelfCenter.z + shelfD / 2 - t / 2},
                                  {2 * w, (y1 - y0) + 0.004f, t}),
                        backMat, white);
            }

            const float upperDy = (top - ys) / glm::length(glm::vec2(hs - ht, top - ys));
            const float lowerDy = (ys - bot) / glm::length(glm::vec2(hs - hb, ys - bot));

            float shelfLevels[] = {-0.50f, 0.15f, 0.65f};

            for (float level : shelfLevels)
            {
                float dy = (level >= ys) ? upperDy : lowerDy;
                float w = halfW(level) - 0.5f * t / dy;
                drawObj(shader, cube,
                        makeModel({shelfCenter.x, shelfCenter.y + level, shelfCenter.z},
                                  {2 * w, t, shelfD - 0.008f}),
                        shelfMat, texWood);
            }
        }

        {

            auto mk = [](glm::vec3 c,
                         glm::vec3 spec = {0.018f, 0.018f, 0.018f},
                         float shin = 8.0f)
            {
                return Material{c * 0.35f, c, spec, shin, glm::vec3(0.0f)};
            };
            auto part = [](const glm::mat4 &base, glm::vec3 p, glm::vec3 s)
            {
                return glm::scale(glm::translate(base, p), s);
            };

            const float z = 4.60f;

            const float s0 = 0.07f;
            const float s1 = 1.10f - 0.50f + 0.035f;
            const float s2 = 1.10f + 0.15f + 0.035f;
            const float s3 = 1.10f + 0.65f + 0.035f;

            Material gold = mk({0.62f, 0.47f, 0.23f});
            Material paper = mk({0.78f, 0.69f, 0.53f});
            Material boneM = mk({0.78f, 0.74f, 0.66f});
            Material voidM = mk({0.01f, 0.01f, 0.01f});
            Material corkM = mk({0.37f, 0.27f, 0.19f});
            Material glassM = mk({0.21f, 0.28f, 0.29f}, {0.62f, 0.62f, 0.70f}, 96.0f);

            struct BookSpec
            {
                float w, h, d;
                glm::vec3 col;
                float tilt;
            };

            auto standBook = [&](float x, float surfY, const BookSpec &b)
            {
                glm::mat4 base = glm::translate(glm::mat4(1.0f), glm::vec3(x, surfY, z));
                base = glm::rotate(base, glm::radians(b.tilt), glm::vec3(0, 0, 1));

                drawObj(shader, cube, part(base, {0, b.h / 2, 0}, {b.w, b.h, b.d}), mk(b.col), white);
                drawObj(shader, cube, part(base, {0, b.h * 0.84f, -b.d / 2 - 0.002f}, {b.w * 1.03f, 0.012f, 0.008f}), gold, white);
                drawObj(shader, cube, part(base, {0, b.h * 0.16f, -b.d / 2 - 0.002f}, {b.w * 1.03f, 0.012f, 0.008f}), gold, white);
                drawObj(shader, cube, part(base, {0, b.h * 0.52f, -b.d / 2 - 0.002f}, {b.w * 0.65f, b.h * 0.28f, 0.006f}), paper, white);
            };

            auto bookRow = [&](float x0, float surfY, std::initializer_list<BookSpec> list)
            {
                float edge = x0;
                for (const BookSpec &b : list)
                {
                    float rad = glm::radians(b.tilt);
                    float cx = (b.tilt > 0.0f)
                                   ? edge + (b.w / 2) * cosf(rad) + b.h * sinf(rad)
                                   : edge + b.w / 2;
                    standBook(cx, surfY, b);
                    edge = cx + (b.w / 2) * cosf(rad) + 0.004f;
                }
                return edge;
            };

            auto flatBook = [&](glm::vec3 pos, float w, float h, float d, glm::vec3 col, float yawDeg)
            {
                glm::mat4 B = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y + h / 2, pos.z));
                B = glm::rotate(B, glm::radians(yawDeg), glm::vec3(0, 1, 0));
                drawObj(shader, cube, glm::scale(B, glm::vec3(w, h, d)), mk(col), white);
                drawObj(shader, cube, part(B, {0, 0, -d / 2 - 0.001f}, {w * 0.94f, h * 0.78f, 0.004f}), paper, white);
            };

            auto drawSkull = [&](glm::vec3 pos, float s, float yawDeg, glm::vec3 eyeCol, bool crystals)
            {
                glm::mat4 S = glm::translate(glm::mat4(1.0f), pos);
                S = glm::rotate(S, glm::radians(yawDeg), glm::vec3(0, 1, 0));
                S = glm::scale(S, glm::vec3(s));

                drawObj(shader, sphere, part(S, {0, 0.20f, 0}, {0.30f, 0.27f, 0.29f}), boneM, white);
                drawObj(shader, cube, part(S, {0, 0.085f, -0.045f}, {0.17f, 0.09f, 0.15f}), boneM, white);
                drawObj(shader, sphere, part(S, {-0.085f, 0.115f, -0.07f}, {0.07f, 0.05f, 0.06f}), boneM, white);
                drawObj(shader, sphere, part(S, {0.085f, 0.115f, -0.07f}, {0.07f, 0.05f, 0.06f}), boneM, white);
                drawObj(shader, cube, part(S, {-0.085f, 0.06f, -0.01f}, {0.03f, 0.09f, 0.06f}), boneM, white);
                drawObj(shader, cube, part(S, {0.085f, 0.06f, -0.01f}, {0.03f, 0.09f, 0.06f}), boneM, white);
                drawObj(shader, cube, part(S, {0, 0.012f, -0.04f}, {0.16f, 0.04f, 0.13f}), boneM, white);

                Material eyeMat = mk(eyeCol);
                for (int side = -1; side <= 1; side += 2)
                {
                    float sx = side * 0.065f;
                    drawObj(shader, sphere, part(S, {sx, 0.16f, -0.12f}, {0.075f, 0.085f, 0.05f}), voidM, white);
                    drawObj(shader, sphere, part(S, {sx, 0.16f, -0.138f}, {0.028f, 0.028f, 0.012f}), eyeMat, white);
                }

                drawObj(shader, sphere, part(S, {0, 0.105f, -0.125f}, {0.035f, 0.055f, 0.02f}), voidM, white);

                for (int i = 0; i < 6; i++)
                {
                    float tx = (i - 2.5f) * 0.027f;
                    drawObj(shader, cube, part(S, {tx, 0.062f, -0.122f}, {0.023f, 0.034f, 0.014f}), boneM, white);
                    drawObj(shader, cube, part(S, {tx, 0.035f, -0.112f}, {0.021f, 0.028f, 0.014f}), boneM, white);
                }
                drawObj(shader, cube, part(S, {0, 0.049f, -0.118f}, {0.16f, 0.006f, 0.01f}), voidM, white);

                if (crystals)
                {
                    Material ameD = mk({0.42f, 0.25f, 0.60f});
                    Material ameL = mk({0.63f, 0.43f, 0.74f});
                    struct Cr
                    {
                        glm::vec3 p, sz;
                        float rz, rx;
                        bool light;
                    };
                    Cr cr[] = {
                        {{0.02f, 0.355f, 0.00f}, {0.040f, 0.11f, 0.040f}, -10.0f, 5.0f, false},
                        {{0.07f, 0.335f, 0.01f}, {0.035f, 0.09f, 0.035f}, -30.0f, 10.0f, true},
                        {{-0.04f, 0.340f, 0.02f}, {0.030f, 0.08f, 0.030f}, 20.0f, -8.0f, false},
                        {{0.10f, 0.300f, 0.02f}, {0.030f, 0.07f, 0.030f}, -55.0f, 0.0f, true},
                        {{0.00f, 0.345f, 0.06f}, {0.035f, 0.09f, 0.035f}, 0.0f, 25.0f, false},
                    };
                    for (const Cr &c : cr)
                    {
                        glm::mat4 C = glm::translate(S, c.p);
                        C = glm::rotate(C, glm::radians(c.rz), glm::vec3(0, 0, 1));
                        C = glm::rotate(C, glm::radians(c.rx), glm::vec3(1, 0, 0));
                        drawObj(shader, cube, glm::scale(C, c.sz), c.light ? ameL : ameD, white);
                    }
                }
            };

            auto labelX = [&](const glm::mat4 &P, glm::vec3 c, float w, float h)
            {
                drawObj(shader, cube, part(P, c, {w, h, 0.005f}), paper, white);
                for (float ang : {45.0f, -45.0f})
                {
                    glm::mat4 X = glm::rotate(glm::translate(P, c + glm::vec3(0, 0, -0.004f)),
                                              glm::radians(ang), glm::vec3(0, 0, 1));
                    drawObj(shader, cube, glm::scale(X, glm::vec3(h * 0.8f, 0.007f, 0.004f)), voidM, white);
                }
            };

            auto drawPotion = [&](glm::vec3 pos, int shape, float s, glm::vec3 col, int label)
            {
                glm::mat4 P = glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(s));
                Material liquid = mk(col, {0.42f, 0.42f, 0.46f}, 64.0f);

                if (shape == 0)
                {
                    drawObj(shader, sphere, part(P, {0, 0.085f, 0}, {0.19f, 0.17f, 0.19f}), liquid, white);
                    drawObj(shader, cylinder, part(P, {0, 0.185f, 0}, {0.060f, 0.13f, 0.060f}), glassM, white);
                    drawObj(shader, cylinder, part(P, {0, 0.150f, 0}, {0.064f, 0.04f, 0.064f}), liquid, white);
                    drawObj(shader, cylinder, part(P, {0, 0.265f, 0}, {0.068f, 0.05f, 0.068f}), corkM, white);
                    if (label == 1)
                        labelX(P, {0, 0.075f, -0.096f}, 0.085f, 0.065f);
                }
                else if (shape == 1)
                {
                    drawObj(shader, cylinder, part(P, {0, 0.100f, 0}, {0.075f, 0.20f, 0.075f}), liquid, white);
                    drawObj(shader, cylinder, part(P, {0, 0.225f, 0}, {0.040f, 0.05f, 0.040f}), glassM, white);
                    drawObj(shader, cylinder, part(P, {0, 0.2675f, 0}, {0.050f, 0.035f, 0.050f}), corkM, white);
                    if (label == 1)
                        labelX(P, {0, 0.10f, -0.040f}, 0.050f, 0.060f);
                }
                else if (shape == 2)
                {
                    for (int k = 0; k < 6; k++)
                    {
                        float dia = 0.15f - k * 0.016f;
                        drawObj(shader, cylinder, part(P, {0, 0.015f + k * 0.03f, 0}, {dia, 0.03f, dia}),
                                (k < 4) ? liquid : glassM, white);
                    }
                    drawObj(shader, cylinder, part(P, {0, 0.215f, 0}, {0.040f, 0.07f, 0.040f}), glassM, white);
                    drawObj(shader, cylinder, part(P, {0, 0.2625f, 0}, {0.048f, 0.025f, 0.048f}), corkM, white);
                    if (label == 1)
                        labelX(P, {0, 0.04f, -0.078f}, 0.060f, 0.045f);
                }
                else
                {
                    drawObj(shader, cylinder, part(P, {0, 0.065f, 0}, {0.13f, 0.13f, 0.13f}), liquid, white);
                    drawObj(shader, cylinder, part(P, {0, 0.140f, 0}, {0.12f, 0.02f, 0.12f}), glassM, white);
                    drawObj(shader, cylinder, part(P, {0, 0.1625f, 0}, {0.14f, 0.025f, 0.14f}), gold, white);
                    drawObj(shader, sphere, part(P, {0, 0.185f, 0}, {0.03f, 0.025f, 0.03f}), gold, white);

                    if (label == 3)
                    {
                        Material eyeW = mk({0.95f, 0.93f, 0.85f});
                        Material iris = mk({0.48f, 0.28f, 0.10f});
                        drawObj(shader, sphere, part(P, {0, 0.065f, -0.045f}, {0.055f, 0.055f, 0.055f}), eyeW, white);
                        drawObj(shader, sphere, part(P, {0, 0.065f, -0.0675f}, {0.028f, 0.028f, 0.012f}), iris, white);
                        drawObj(shader, sphere, part(P, {0, 0.065f, -0.0725f}, {0.012f, 0.012f, 0.008f}), voidM, white);
                    }
                    else if (label == 1)
                        labelX(P, {0, 0.065f, -0.068f}, 0.070f, 0.060f);
                }
            };

            drawPotion({0.24f, s0, z}, 0, 1.0f, {0.55f, 0.10f, 0.22f}, 1);
            drawPotion({0.395f, s0, z}, 1, 1.0f, {0.06f, 0.44f, 0.30f}, 0);
            drawPotion({0.525f, s0, z}, 2, 1.1f, {0.73f, 0.34f, 0.10f}, 1);

            flatBook({0.78f, s0, z}, 0.30f, 0.045f, 0.21f, {0.30f, 0.10f, 0.40f}, -8.0f);
            drawSkull({0.78f, s0 + 0.045f, z}, 1.0f, -15.0f, {0.49f, 0.27f, 0.64f}, true);

            drawPotion({1.00f, s0, z}, 3, 0.7f, {0.65f, 0.49f, 0.15f}, 1);

            drawSkull({0.32f, s1, z}, 0.80f, 12.0f,
                      {0.18f, 0.19f, 0.21f}, false);

            bookRow(0.74f, s1, {
                                   {0.050f, 0.30f, 0.20f, {0.55f, 0.06f, 0.08f}, 0.0f},
                                   {0.060f, 0.26f, 0.19f, {0.13f, 0.40f, 0.22f}, 0.0f},
                                   {0.045f, 0.32f, 0.21f, {0.59f, 0.12f, 0.26f}, 0.0f},
                                   {0.050f, 0.28f, 0.18f, {0.42f, 0.28f, 0.60f}, 12.0f},
                               });
            drawPotion({1.08f, s1, z}, 1, 1.0f, {0.57f, 0.07f, 0.13f}, 1);

            bookRow(0.44f, s2, {
                                   {0.050f, 0.30f, 0.19f, {0.06f, 0.40f, 0.30f}, 0.0f},
                                   {0.060f, 0.26f, 0.20f, {0.52f, 0.12f, 0.24f}, 0.0f},
                                   {0.045f, 0.31f, 0.17f, {0.68f, 0.43f, 0.12f}, 0.0f},
                                   {0.050f, 0.27f, 0.18f, {0.08f, 0.15f, 0.45f}, 12.0f},
                               });

            {
                // Bola de cristal com emissao roxa discreta e aura propria.
                const float crystalX = 0.83f;
                glm::mat4 base = glm::translate(glm::mat4(1.0f),
                                                glm::vec3(crystalX, s2, z));

                Material darkVelvet = mk({0.28f, 0.13f, 0.32f});
                Material brass = mk({0.65f, 0.48f, 0.23f});
                Material crystal = mk({0.31f, 0.19f, 0.64f},
                                      {0.62f, 0.55f, 0.85f}, 96.0f);

                const float crystalPulse = 0.84f + 0.16f * std::sin(now * 1.5f);
                crystal.emissive = glm::vec3(0.13f, 0.07f, 0.26f) * crystalPulse;

                drawObj(shader, cylinder,
                        part(base, {0.0f, 0.004f, 0.0f}, {0.25f, 0.008f, 0.25f}),
                        darkVelvet, white);
                drawObj(shader, cylinder,
                        part(base, {0.0f, 0.022f, 0.0f}, {0.14f, 0.035f, 0.14f}),
                        brass, white);
                drawObj(shader, cylinder,
                        part(base, {0.0f, 0.049f, 0.0f}, {0.095f, 0.023f, 0.095f}),
                        brass, white);

                drawObj(shader, sphere,
                        part(base, {0.0f, 0.170f, 0.0f}, {0.24f, 0.24f, 0.24f}),
                        crystal, white);
            }

            flatBook({1.10f, s2, z}, 0.22f, 0.045f, 0.17f, {0.09f, 0.36f, 0.31f}, 0.0f);
            flatBook({1.11f, s2 + 0.045f, z}, 0.20f, 0.040f, 0.16f, {0.52f, 0.12f, 0.24f}, 10.0f);
            flatBook({1.09f, s2 + 0.085f, z}, 0.18f, 0.040f, 0.15f, {0.68f, 0.43f, 0.12f}, -8.0f);
            drawPotion({1.10f, s2 + 0.125f, z}, 3, 0.7f, {0.52f, 0.10f, 0.25f}, 3);

            flatBook({0.28f, s3, z}, 0.26f, 0.04f, 0.19f, {0.70f, 0.33f, 0.10f}, 6.0f);
            drawSkull({0.28f, s3 + 0.04f, z}, 0.55f, -25.0f, {0.12f, 0.49f, 0.26f}, false);

            drawPotion({0.47f, s3, z}, 1, 1.00f, {0.46f, 0.26f, 0.70f}, 1);
            drawPotion({0.62f, s3, z}, 0, 0.85f, {0.73f, 0.34f, 0.10f}, 0);
            drawPotion({0.80f, s3, z}, 2, 0.90f, {0.15f, 0.37f, 0.65f}, 1);
            drawPotion({0.97f, s3, z}, 3, 0.80f, {0.08f, 0.45f, 0.23f}, 3);
        }

        shader.use();
        shader.setFloat("fogDensity", 0.0f);
        shader.setFloat("isMountain", 0.0f);

        // Exterior sem a neblina vermelha da sala.
        Material grassMat{
            {0.03f, 0.05f, 0.03f},
            {0.14f, 0.22f, 0.17f},
            {0.02f, 0.02f, 0.02f},
            2.0f,
            {0.00f, 0.01f, 0.00f}};

        Material mountainMat{
            {0.025f, 0.025f, 0.040f},
            {0.20f, 0.22f, 0.30f},
            {0.0f, 0.0f, 0.0f},
            1.0f,
            {0.025f, 0.030f, 0.045f}};

        Material moonMat{
            {0.85f, 0.85f, 0.85f},
            {0.80f, 0.80f, 0.80f},
            {0.03f, 0.03f, 0.03f},
            8.0f,
            {0.28f, 0.28f, 0.23f}};

        drawObj(shader, plane,
                makeModel(
                    {0.0f, 0.0f, -30.0f},
                    {110.0f, 1.0f, 50.0f}),
                grassMat, white);

        drawObj(shader, sphere,
                makeModel(moonPos, {1.8f, 1.8f, 1.8f}),
                moonMat, texMoon);

        shader.setFloat("isMountain", 1.0f);
        drawObj(shader, mountains,
                glm::mat4(1.0f),
                mountainMat, white);
        shader.setFloat("isMountain", 0.0f);

        {
            // Lapides externas recebem contraluz; a nevoa e desenhada depois.
            graveyardShader.use();
            graveyardShader.setMat4("projection", glm::perspective(
                                                      glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
            graveyardShader.setMat4("view", camera.getViewMatrix());
            graveyardShader.setVec3("viewPos", camera.position);
            graveyardShader.setVec3("moonLightDirection", moonDir);
            graveyardShader.setVec3("moonLightColor", glm::vec3(0.36f, 0.48f, 0.72f));
            graveyardShader.setInt("tex", 0);

            struct ExteriorTomb
            {
                glm::vec3 base;
                float width, height, rotY;
                glm::vec3 stoneColor;
            };

            const ExteriorTomb exteriorTombs[3] = {
                {{-3.35f, -0.12f, -10.6f}, 0.60f, 1.45f, -8.0f, {0.22f, 0.24f, 0.29f}},
                {{-1.75f, -0.15f, -12.1f}, 0.68f, 1.70f, 5.0f, {0.25f, 0.26f, 0.31f}},
                {{-0.15f, -0.11f, -11.0f}, 0.56f, 1.55f, -5.0f, {0.20f, 0.23f, 0.27f}}};

            for (const ExteriorTomb &t : exteriorTombs)
            {
                Material stone{
                    t.stoneColor * 0.10f,
                    t.stoneColor,
                    {0.0f, 0.0f, 0.0f},
                    1.0f,
                    {0.0f, 0.0f, 0.0f}};

                glm::mat4 base = glm::translate(glm::mat4(1.0f), t.base);
                base = glm::rotate(base, glm::radians(t.rotY), glm::vec3(0, 1, 0));

                drawObj(graveyardShader, cube,
                        glm::scale(glm::translate(base, {0, t.height * 0.5f, 0}),
                                   {t.width, t.height, 0.17f}),
                        stone, white);

                glm::mat4 cap = glm::translate(base, {0, t.height, 0});
                cap = glm::rotate(cap, glm::radians(90.0f), glm::vec3(1, 0, 0));
                drawObj(graveyardShader, cylinder,
                        glm::scale(cap, {t.width, 0.18f, t.width}),
                        stone, white);
            }

            shader.use();
        }

        {
            struct EyePair
            {
                glm::vec3 center;
                float spacing;
                float size;
                float blinkPeriod;
                float blinkOffset;
            };
            const EyePair pairs[3] = {
                {{-2.60f, 1.18f, -13.60f}, 0.32f, 0.44f, 8.50f, 0.0f},
                {{-0.95f, 1.40f, -14.80f}, 0.30f, 0.40f, 7.30f, 2.5f},
                {{-3.45f, 1.65f, -17.20f}, 0.35f, 0.42f, 9.10f, 4.3f}};

            const glm::vec2 windowCenter(-1.80f, -4.85f);
            const glm::vec2 playerXZ(camera.position.x, camera.position.z);
            const float distToWindow = glm::length(playerXZ - windowCenter);
            const float distanceFade = glm::smoothstep(0.75f, 2.60f, distToWindow);
            const float scaredFade = glfwGetTime() < eyesHiddenUntil ? 0.0f : 1.0f;
            const float overallVisibility = distanceFade * scaredFade;

            if (overallVisibility > 0.001f)
            {
                const GLboolean blendBefore = glIsEnabled(GL_BLEND);
                const GLboolean cullBefore = glIsEnabled(GL_CULL_FACE);
                GLboolean depthWriteBefore = GL_TRUE;
                glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteBefore);

                GLint blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha;
                glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRGB);
                glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRGB);
                glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
                glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstAlpha);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);

                eyesGlowShader.use();
                eyesGlowShader.setMat4("projection", glm::perspective(
                                                         glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
                eyesGlowShader.setMat4("view", camera.getViewMatrix());
                eyesGlowShader.setVec2("uvScale", glm::vec2(1.0f));

                for (const EyePair &pair : pairs)
                {

                    const float cycle = std::fmod(now + pair.blinkOffset, pair.blinkPeriod);
                    const float blink = (cycle > pair.blinkPeriod - 2.70f &&
                                         cycle < pair.blinkPeriod - 2.55f)
                                            ? 0.0f
                                            : 1.0f;
                    const float breathe = 0.93f + 0.07f * std::sin(now * 1.6f + pair.blinkOffset);
                    const float visibility = overallVisibility * blink * breathe;
                    if (visibility <= 0.001f)
                        continue;
                    eyesGlowShader.setFloat("visibility", visibility);

                    for (int eye = 0; eye < 2; ++eye)
                    {
                        const float offsetX = (eye == 0 ? -0.5f : 0.5f) * pair.spacing;
                        const glm::vec3 eyePos = pair.center + glm::vec3(offsetX, 0.0f, 0.0f);
                        glm::mat4 eyeModel = glm::inverse(camera.getViewMatrix());
                        eyeModel[3] = glm::vec4(eyePos, 1.0f);
                        eyeModel = glm::rotate(eyeModel, glm::radians(90.0f),
                                               glm::vec3(1.0f, 0.0f, 0.0f));
                        eyeModel = glm::scale(eyeModel,
                                              glm::vec3(pair.size, 1.0f, pair.size));
                        eyesGlowShader.setMat4("model", eyeModel);
                        plane.draw();
                    }
                }

                glDepthMask(depthWriteBefore);
                if (cullBefore)
                    glEnable(GL_CULL_FACE);
                if (!blendBefore)
                    glDisable(GL_BLEND);
                glBlendFuncSeparate(blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha);
                shader.use();
            }
        }

        shader.setFloat("fogDensity", fogDen);

        drawObj(shader, cube, makeModel({0, 1.5f, 5}, {10, 3, 0.2f}), Mat::wallHalloween(), texWall, {3, 1}, nrmWall);
        drawObj(shader, cube, makeModel({-5, 1.5f, 0}, {0.2f, 3, 10}), Mat::wallHalloween(), texWall, {3, 1}, nrmWall);

        drawObj(shader, cube, makeModel({5, 1.5f, -2.0f}, {0.2f, 3, 6}), Mat::wallHalloween(), texWall, {2, 1}, nrmWall);
        drawObj(shader, cube, makeModel({5, 1.5f, 3.5f}, {0.2f, 3, 3}), Mat::wallHalloween(), texWall, {1, 1}, nrmWall);
        drawObj(shader, cube, makeModel({5, 2.6f, 1.5f}, {0.2f, 0.8f, 1}), Mat::wallHalloween(), texWall, glm::vec2(1.0f), nrmWall);

        glm::mat4 doorBase(1.0f);
        doorBase = glm::translate(doorBase, glm::vec3(5.0f, 0.0f, 1.0f));
        doorBase = glm::rotate(doorBase, glm::radians(doorAngle), glm::vec3(0, 1, 0));
        glm::mat4 panel = glm::translate(doorBase, glm::vec3(0.0f, 1.1f, 0.5f));
        panel = glm::scale(panel, glm::vec3(0.08f, 2.2f, 1.0f));
        drawObj(shader, cube, panel, Mat::wood(), texWood);
        glm::mat4 handle = glm::translate(doorBase, glm::vec3(-0.07f, 1.0f, 0.88f));
        handle = glm::scale(handle, glm::vec3(0.08f));
        drawObj(shader, sphere, handle, Mat::metal(), white);

        drawObj(shader, cube, makeModel({0, 0.75f, 0}, {1.6f, 0.1f, 0.9f}), Mat::wood(), texWood);
        float lx[] = {-0.7f, 0.7f}, lz[] = {-0.35f, 0.35f};
        for (float x : lx)
            for (float z : lz)
                drawObj(shader, cube, makeModel({x, 0.35f, z}, {0.1f, 0.7f, 0.1f}), Mat::wood(), texWood);

        drawObj(shader, cylinder, makeModel({0, 0.825f, 0}, {0.25f, 0.05f, 0.25f}), Mat::metal(), white);
        drawObj(shader, cylinder, makeModel({0, 1.05f, 0}, {0.04f, 0.4f, 0.04f}), Mat::metal(), white);
        glm::vec3 glow = lampOn ? glm::vec3(1.0f, 0.6f, 0.2f) : glm::vec3(0.0f);
        drawObj(shader, sphere, makeModel({0, 1.3f, 0}, {0.2f, 0.2f, 0.2f}), Mat::glowing(glow), white);

        {

            auto mk = [](glm::vec3 c, glm::vec3 spec = {0.05f, 0.05f, 0.05f})
            {
                return Material{c * 0.35f, c, spec, 8.0f, glm::vec3(0.0f)};
            };

            const float CW = 0.13f, CL = 0.21f, CT = 0.006f;
            auto flat = [&](const glm::mat4 &F, float x, float z, float sx, float sz, int layer,
                            const Material &m, bool disc = false, float rot = 0.0f)
            {
                glm::mat4 M = glm::translate(F, glm::vec3(x, CT / 2 + 0.0006f * layer, z));
                M = glm::rotate(M, glm::radians(rot), glm::vec3(0, 1, 0));
                M = glm::scale(M, glm::vec3(sx, 0.0003f, sz));
                drawObj(shader, disc ? cylinder : cube, M, m, white);
            };

            Material gold = mk({0.85f, 0.68f, 0.22f}, {0.30f, 0.22f, 0.05f});
            Material bone = mk({0.90f, 0.86f, 0.74f}, {0.25f, 0.24f, 0.20f});
            Material dark = mk({0.01f, 0.01f, 0.01f});
            Material cardEdge = mk({0.75f, 0.58f, 0.18f});

            auto drawBack = [&](const glm::mat4 &F)
            {
                Material pan = mk({0.20f, 0.05f, 0.30f});
                flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                flat(F, 0, 0, 0.065f, 0.065f, 1, gold, true);
                flat(F, 0, 0, 0.052f, 0.052f, 2, pan, true);
                for (int k = 0; k < 4; k++)
                    flat(F, 0, 0, 0.050f, 0.007f, 3, gold, false, k * 45.0f);
                flat(F, 0, -0.080f, 0.016f, 0.016f, 1, gold, true);
                flat(F, 0, 0.080f, 0.016f, 0.016f, 1, gold, true);
            };

            auto drawFace = [&](const glm::mat4 &F, int id)
            {
                switch (id)
                {
                case 0:
                {
                    Material pan = mk({0.06f, 0.08f, 0.28f});
                    flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                    flat(F, -0.005f, -0.010f, 0.080f, 0.080f, 1, gold, true);
                    flat(F, 0.018f, -0.020f, 0.072f, 0.072f, 2, pan, true);
                    flat(F, 0.030f, -0.075f, 0.012f, 0.012f, 1, gold, true);
                    flat(F, -0.035f, 0.050f, 0.010f, 0.010f, 1, gold, true);
                    flat(F, 0.035f, 0.040f, 0.008f, 0.008f, 1, gold, true);
                    break;
                }
                case 1:
                {
                    Material pan = mk({0.05f, 0.04f, 0.06f});
                    flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                    flat(F, 0, 0.005f, 0.100f, 0.010f, 1, bone, false, 45.0f);
                    flat(F, 0, 0.005f, 0.100f, 0.010f, 1, bone, false, -45.0f);
                    flat(F, 0, -0.020f, 0.070f, 0.070f, 2, bone, true);
                    flat(F, 0, 0.025f, 0.042f, 0.026f, 2, bone);
                    flat(F, -0.015f, -0.025f, 0.017f, 0.017f, 3, dark, true);
                    flat(F, 0.015f, -0.025f, 0.017f, 0.017f, 3, dark, true);
                    flat(F, 0, 0.000f, 0.009f, 0.009f, 3, dark, false, 45.0f);
                    break;
                }
                case 2:
                {
                    Material pan = mk({0.04f, 0.14f, 0.08f});
                    Material orange = mk({1.0f, 0.50f, 0.10f}, {0.55f, 0.22f, 0.02f});
                    flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                    flat(F, 0, 0.0f, 0.090f, 0.080f, 1, orange, true);
                    flat(F, 0, -0.052f, 0.012f, 0.026f, 2, mk({0.20f, 0.45f, 0.10f}));
                    flat(F, -0.020f, -0.010f, 0.016f, 0.016f, 3, dark, false, 45.0f);
                    flat(F, 0.020f, -0.010f, 0.016f, 0.016f, 3, dark, false, 45.0f);
                    flat(F, 0, 0.027f, 0.052f, 0.010f, 3, dark);
                    break;
                }
                case 3:
                {
                    Material pan = mk({0.20f, 0.10f, 0.35f});
                    Material ghost = mk({0.95f, 0.95f, 1.0f}, {0.30f, 0.30f, 0.35f});
                    flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                    flat(F, 0, -0.025f, 0.070f, 0.070f, 1, ghost, true);
                    flat(F, 0, 0.010f, 0.070f, 0.070f, 1, ghost);
                    for (int k = -1; k <= 1; k++)
                        flat(F, k * 0.0235f, 0.045f, 0.024f, 0.024f, 1, ghost, true);
                    flat(F, -0.014f, -0.022f, 0.012f, 0.012f, 2, dark, true);
                    flat(F, 0.014f, -0.022f, 0.012f, 0.012f, 2, dark, true);
                    flat(F, 0, 0.002f, 0.013f, 0.016f, 2, dark, true);
                    break;
                }
                case 4:
                {
                    Material pan = mk({0.02f, 0.04f, 0.15f});
                    flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                    flat(F, 0, 0, 0.012f, 0.110f, 1, gold);
                    flat(F, 0, 0, 0.110f, 0.012f, 1, gold);
                    flat(F, 0, 0, 0.075f, 0.008f, 1, gold, false, 45.0f);
                    flat(F, 0, 0, 0.075f, 0.008f, 1, gold, false, -45.0f);
                    flat(F, 0, 0, 0.022f, 0.022f, 2, mk({1, 1, 1}, {0.6f, 0.6f, 0.6f}), true);
                    flat(F, 0.040f, -0.075f, 0.010f, 0.010f, 1, gold, true);
                    flat(F, -0.040f, 0.075f, 0.010f, 0.010f, 1, gold, true);
                    break;
                }
                default:
                {
                    Material pan = mk({0.30f, 0.04f, 0.08f});
                    flat(F, 0, 0, CW - 0.012f, CL - 0.012f, 0, pan);
                    flat(F, 0, -0.050f, 0.052f, 0.052f, 1, gold, true);
                    flat(F, 0, -0.050f, 0.028f, 0.028f, 2, pan, true);
                    flat(F, 0, 0.020f, 0.012f, 0.095f, 1, gold);
                    flat(F, 0.017f, 0.050f, 0.022f, 0.010f, 1, gold);
                    flat(F, 0.017f, 0.030f, 0.022f, 0.010f, 1, gold);
                    break;
                }
                }

                flat(F, 0, 0.088f, 0.070f, 0.008f, 1, gold);
            };

            bool allDown = true;
            for (int i = 0; i < 3; i++)
            {
                float target = (i < cardStage && !cardsShuffling) ? 1.0f : 0.0f;
                cardFlip[i] = approach(cardFlip[i], target, deltaTime * 1.8f);
                if (cardFlip[i] > 0.0f)
                    allDown = false;
            }
            if (cardsShuffling && allDown)
            {
                shuffleDeck();
                cardsShuffling = false;
            }

            const float cardX[3] = {0.30f, 0.45f, 0.60f};
            const float cardYaw[3] = {8.0f, 0.0f, -8.0f};
            const float PI = 3.14159265f;

            for (int i = 0; i < 3; i++)
            {
                float f = cardFlip[i];
                float ease = f * f * (3.0f - 2.0f * f);
                float lift = std::sin(ease * PI) * 0.10f;

                glm::mat4 C = glm::translate(glm::mat4(1.0f),
                                             glm::vec3(cardX[i], 0.80f + CT / 2 + 0.0006f + lift, 0.0f));
                C = glm::rotate(C, glm::radians(cardYaw[i]), glm::vec3(0, 1, 0));
                C = glm::rotate(C, ease * PI, glm::vec3(0, 0, 1));

                drawObj(shader, cube, glm::scale(C, glm::vec3(CW, CT, CL)), cardEdge, white);
                drawBack(C);

                drawFace(glm::rotate(C, PI, glm::vec3(0, 0, 1)), cardPick[i]);
            }
        }

        drawObj(shader, cube, makeModel({2.5f, 0.3f, -4.6f}, {1.6f, 0.6f, 0.5f}), Mat::wood(), texWood);
        drawObj(shader, cube, makeModel({2.5f, 1.2f, -4.85f}, {1.4f, 0.8f, 0.08f}), tvFrame, white);
        glm::vec3 screen = glm::vec3(0.2f, 0.5f, 0.95f) * flicker;
        drawObj(shader, cube, makeModel({2.5f, 1.2f, -4.80f}, {1.3f, 0.7f, 0.02f}),
                tvOn ? Mat::glowing(screen) : tvOff, white);

        glm::vec3 fanPos(-2.5f, 2.75f, -2.5f);
        drawObj(shader, cylinder, makeModel({-2.5f, 2.9f, -2.5f}, {0.04f, 0.2f, 0.04f}), Mat::metal(), white);
        drawObj(shader, cylinder, makeModel(fanPos, {0.25f, 0.12f, 0.25f}), Mat::metal(), white);
        glm::mat4 fanBase = glm::translate(glm::mat4(1.0f), fanPos);
        fanBase = glm::rotate(fanBase, glm::radians(fanAngle), glm::vec3(0, 1, 0));
        for (int i = 0; i < 4; i++)
        {
            glm::mat4 blade = glm::rotate(fanBase, glm::radians(i * 90.0f), glm::vec3(0, 1, 0));
            blade = glm::translate(blade, glm::vec3(0.5f, 0.0f, 0.0f));
            blade = glm::scale(blade, glm::vec3(0.8f, 0.02f, 0.15f));
            drawObj(shader, cube, blade, Mat::black(), white);
        }

        pumpkin(BIG_PUMPKIN, 1.6f, 55.0f, pumpkinOn, flickerP);
        for (int i = 0; i < 4; ++i)
        {
            const PumpkinInfo &p = SMALL_PUMPKINS[i];

            const float fl = 0.86f + 0.14f * std::sin(now * (6.0f + i * 0.6f) + i * 1.7f);
            pumpkin(p.pos, p.scale, p.rotY, pumpkinOn, fl);
        }

        {
            glm::mat4 sb = glm::translate(glm::mat4(1.0f), glm::vec3(-0.45f, 0.8f, 0.2f));
            sb = glm::rotate(sb, glm::radians(25.0f), glm::vec3(0, 1, 0));
            drawObj(shader, sphere, glm::scale(glm::translate(sb, {0, 0.07f, 0}), {0.14f, 0.13f, 0.14f}), Mat::bone(), white);
            drawObj(shader, cube, glm::scale(glm::translate(sb, {0, 0.02f, 0.025f}), {0.08f, 0.04f, 0.08f}), Mat::bone(), white);
            for (int sx = -1; sx <= 1; sx += 2)
                drawObj(shader, sphere, glm::scale(glm::translate(sb, {sx * 0.03f, 0.08f, 0.055f}), glm::vec3(0.038f)), Mat::black(), white);
            drawObj(shader, cube, glm::scale(glm::translate(sb, {0, 0.05f, 0.068f}), {0.015f, 0.025f, 0.015f}), Mat::black(), white);
        }

        const glm::vec2 *candles = CANDLE_XZ;
        for (int i = 0; i < 3; i++)
        {
            float fl = 0.8f + 0.2f * std::sin(now * (8.0f + i * 1.7f) + i * 2.0f);
            drawObj(shader, cylinder, makeModel({candles[i].x, 0.87f, candles[i].y}, {0.05f, 0.14f, 0.05f}), Mat::wax(), white);
            if (candleLit[i])
                drawObj(shader, sphere, makeModel({candles[i].x, 0.98f, candles[i].y}, {0.035f, 0.07f * fl + 0.03f, 0.035f}),
                        Mat::glowing(glm::vec3(1.0f, 0.7f, 0.2f) * fl), white);
            else
                drawObj(shader, cylinder, makeModel({candles[i].x, 0.96f, candles[i].y}, {0.008f, 0.04f, 0.008f}), Mat::black(), white);
        }

        {
            glm::vec3 cp(3.3f, 0.0f, -2.0f);
            drawObj(shader, sphere, makeModel(cp + glm::vec3(0, 0.30f, 0), {0.85f, 0.60f, 0.85f}), Mat::black(), white);
            drawObj(shader, cylinder, makeModel(cp + glm::vec3(0, 0.55f, 0), {0.78f, 0.20f, 0.78f}), Mat::black(), white);
            drawObj(shader, cylinder, makeModel(cp + glm::vec3(0, 0.66f, 0), {0.62f, 0.02f, 0.62f}),
                    Mat::glowing((potionDone ? glm::vec3(0.7f, 0.25f, 1.0f) : glm::vec3(0.2f, 0.9f, 0.15f)) * (0.8f + 0.2f * std::sin(now * 3.0f))), white);

            for (int i = 0; i < 4; i++)
            {
                float t = std::fmod(now * 0.45f + i * 0.25f, 1.0f);
                float a = i * 1.9f;
                glm::vec3 bp = cp + glm::vec3(0.12f * std::cos(a), 0.68f + t * 0.45f, 0.12f * std::sin(a));
                float sz = 0.07f * (1.0f - t * 0.6f);
                drawObj(shader, sphere, makeModel(bp, glm::vec3(sz)), Mat::glowing(potionDone ? glm::vec3(0.8f, 0.4f, 1.0f) : glm::vec3(0.3f, 1.0f, 0.2f)), white);
            }

            for (int i = 0; i < 3; i++)
            {
                float a = i * 2.0944f + 0.5f;
                drawObj(shader, cylinder, makeModel(cp + glm::vec3(0.28f * std::cos(a), 0.05f, 0.28f * std::sin(a)), {0.07f, 0.12f, 0.07f}), Mat::black(), white);
            }
        }

        tomb({-4.55f, 0.0f, -0.6f}, 90.0f, 0.65f);
        tomb({-4.45f, 0.0f, 0.5f}, 80.0f, 0.50f);
        tomb({-4.55f, 0.0f, 1.5f}, 100.0f, 0.75f);

        web({-4.89f, 2.98f, -4.89f}, {1, 0, 0}, {0, -1, 0}, 1.3f);
        web({4.89f, 2.98f, -4.89f}, {-1, 0, 0}, {0, -1, 0}, 1.3f);
        web({-4.89f, 2.98f, 4.89f}, {0, 0, -1}, {0, -1, 0}, 1.1f);
        web({-4.89f, 2.98f, -4.89f}, {0, 0, 1}, {0, -1, 0}, 0.9f);

        {
            glm::vec3 top(1.8f, 2.99f, 1.2f);
            glm::vec3 body = top + glm::vec3(0.0f, -(0.9f + 0.4f * (0.5f + 0.5f * std::sin(now * 0.8f))), 0.0f);
            line(top, body + glm::vec3(0, 0.08f, 0), 0.004f, Mat::web());
            glm::mat4 sb = glm::translate(glm::mat4(1.0f), body);
            sb = glm::rotate(sb, now * 0.6f, glm::vec3(0, 1, 0));
            drawObj(shader, sphere, glm::scale(sb, {0.14f, 0.11f, 0.18f}), Mat::black(), white);
            drawObj(shader, sphere, glm::scale(glm::translate(sb, {0, 0, 0.13f}), glm::vec3(0.09f)), Mat::black(), white);
            for (int sx = -1; sx <= 1; sx += 2)
                drawObj(shader, sphere, glm::scale(glm::translate(sb, {sx * 0.025f, 0.02f, 0.19f}), glm::vec3(0.02f)),
                        Mat::glowing(glm::vec3(1.0f, 0.0f, 0.0f)), white);
            for (int i = 0; i < 4; i++)
                for (int sx = -1; sx <= 1; sx += 2)
                {
                    float zz = (i - 1.5f) * 0.07f;
                    glm::vec3 knee = glm::vec3(sb * glm::vec4(sx * 0.17f, 0.10f, zz, 1.0f));
                    glm::vec3 foot = glm::vec3(sb * glm::vec4(sx * 0.30f, -0.12f, zz * 1.6f, 1.0f));
                    glm::vec3 root = glm::vec3(sb * glm::vec4(sx * 0.06f, 0.0f, zz * 0.6f, 1.0f));
                    line(root, knee, 0.012f, Mat::black());
                    line(knee, foot, 0.010f, Mat::black());
                }
        }

        for (int i = 0; i < 3; i++)
        {
            float a = now * (0.8f + 0.2f * i) + i * 2.1f;
            float r = 1.2f + 0.6f * i;
            glm::vec3 bp(std::cos(a) * r, 2.3f + 0.15f * std::sin(now * 3.0f + i), std::sin(a) * r);
            float yaw = std::atan2(-std::cos(a), -std::sin(a));
            glm::mat4 bb = glm::translate(glm::mat4(1.0f), bp);
            bb = glm::rotate(bb, yaw, glm::vec3(0, 1, 0));
            drawObj(shader, sphere, glm::scale(bb, {0.14f, 0.09f, 0.09f}), Mat::black(), white);
            float flap = 40.0f * std::sin(now * 14.0f + i);
            for (int sd = -1; sd <= 1; sd += 2)
            {
                glm::mat4 wg = glm::rotate(bb, glm::radians(sd * flap), glm::vec3(1, 0, 0));
                wg = glm::translate(wg, glm::vec3(0, 0, sd * 0.17f));
                wg = glm::scale(wg, glm::vec3(0.14f, 0.01f, 0.32f));
                drawObj(shader, cube, wg, Mat::black(), white);
            }
        }

        for (int gi = 0; gi < 3; ++gi)
        {
            glm::vec3 gp = GHOST_POS[gi] +
                           glm::vec3(0.0f, 0.15f * std::sin(now * 1.5f + gi * 1.8f), 0.0f);
            float yaw = std::atan2(camera.position.x - gp.x, camera.position.z - gp.z);
            glm::mat4 gb = glm::translate(glm::mat4(1.0f), gp);
            gb = glm::rotate(gb, yaw, glm::vec3(0, 1, 0));

            const glm::vec3 color = GHOST_COLORS[gi];
            // O espirito recupera seu brilho colorido somente apos ser libertado.
            const float emission = ghostFound[gi] ? 0.90f : 0.14f;
            Material gm{
                color * 0.20f,
                color * 0.68f,
                glm::vec3(0.025f),
                10.0f,
                color * emission};

            drawObj(shader, sphere, glm::scale(gb, {0.55f, 0.65f, 0.55f}), gm, white);
            drawObj(shader, cylinder,
                    glm::scale(glm::translate(gb, {0, -0.2f, 0}), {0.55f, 0.45f, 0.55f}), gm, white);
            for (int i = 0; i < 8; i++)
            {
                float a = i * 2.0f * PI_F / 8.0f;
                float wv = 0.03f * std::sin(now * 3.0f + i * 1.7f + gi);
                drawObj(shader, sphere,
                        glm::scale(glm::translate(gb,
                                                  {0.26f * std::cos(a), -0.43f + wv, 0.26f * std::sin(a)}),
                                   glm::vec3(0.15f)),
                        gm, white);
            }
            // Olhos vermelhos enquanto preso; pretos apos libertar o espirito.
            Material ghostEyes = ghostFound[gi]
                                     ? Mat::black()
                                     : Mat::glowing(glm::vec3(0.95f, 0.025f, 0.015f));
            for (int sx = -1; sx <= 1; sx += 2)
            {
                drawObj(shader, sphere,
                        glm::scale(glm::translate(gb, {sx * 0.09f, 0.08f, 0.24f}),
                                   {0.07f, 0.10f, 0.05f}),
                        ghostEyes, white);
                glm::mat4 arm = glm::translate(gb,
                                               {sx * 0.30f, -0.05f + 0.05f * std::sin(now * 2.0f + sx + gi), 0.06f});
                arm = glm::rotate(arm, glm::radians(sx * 25.0f), glm::vec3(0, 0, 1));
                drawObj(shader, sphere, glm::scale(arm, {0.10f, 0.22f, 0.10f}), gm, white);
            }
            drawObj(shader, sphere,
                    glm::scale(glm::translate(gb, {0, -0.08f, 0.25f}),
                               {0.08f, 0.12f, 0.04f}),
                    Mat::black(), white);
        }

        for (int i = 0; i < 3; i++)
        {
            if (candyGot[i])
                continue;
            glm::vec3 cp = CANDY_POS[i] + glm::vec3(0, 0.05f * std::sin(now * 2.0f + i), 0);
            glm::mat4 cb = glm::rotate(glm::translate(glm::mat4(1.0f), cp), now * 1.5f + i, glm::vec3(0, 1, 0));
            Material body = Mat::glowing(glm::vec3(1.0f, 0.45f, 0.05f) * 0.8f);
            drawObj(shader, sphere, glm::scale(cb, {0.16f, 0.09f, 0.09f}), body, white);
            for (int sx = -1; sx <= 1; sx += 2)
            {
                glm::mat4 w = glm::translate(cb, {sx * 0.11f, 0, 0});
                w = glm::rotate(w, glm::radians(45.0f), glm::vec3(1, 0, 0));
                drawObj(shader, cube, glm::scale(w, {0.05f, 0.07f, 0.07f}), Mat::glowing(glm::vec3(0.6f, 0.2f, 0.9f) * 0.8f), white);
            }
        }

        if (potionDone && !keyGot)
        {
            glm::vec3 kp = KEY_POS + glm::vec3(0, 0.08f * std::sin(now * 2.5f), 0);
            glm::mat4 kb = glm::rotate(glm::translate(glm::mat4(1.0f), kp), now * 1.2f, glm::vec3(0, 1, 0));
            Material gold{{0.30f, 0.22f, 0.05f}, {0.80f, 0.60f, 0.10f}, {1.0f, 0.9f, 0.4f}, 64.0f, {0.35f, 0.25f, 0.02f}};
            glm::mat4 head = glm::rotate(glm::translate(kb, {0, 0.17f, 0}), glm::radians(90.0f), glm::vec3(1, 0, 0));
            drawObj(shader, cylinder, glm::scale(head, {0.14f, 0.03f, 0.14f}), gold, white);
            drawObj(shader, cylinder, glm::scale(glm::translate(head, {0, 0, 0}), {0.06f, 0.04f, 0.06f}), Mat::black(), white);
            drawObj(shader, cube, glm::scale(glm::translate(kb, {0, -0.03f, 0}), {0.035f, 0.32f, 0.035f}), gold, white);
            drawObj(shader, cube, glm::scale(glm::translate(kb, {0.05f, -0.14f, 0}), {0.08f, 0.035f, 0.035f}), gold, white);
            drawObj(shader, cube, glm::scale(glm::translate(kb, {0.04f, -0.08f, 0}), {0.06f, 0.035f, 0.035f}), gold, white);
        }

        {
            GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
            GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
            GLboolean depthWriteWasEnabled = GL_TRUE;
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteWasEnabled);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);

            groundFogShader.use();
            groundFogShader.setMat4("view", camera.getViewMatrix());
            groundFogShader.setMat4("projection",
                                    glm::perspective(glm::radians(60.0f),
                                                     (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f));
            groundFogShader.setVec2("uvScale", glm::vec2(1.0f));
            groundFogShader.setFloat("time", now);

            const float fogHeights[4] = {0.10f, 0.27f, 0.45f, 0.65f};
            const float fogOpacities[4] = {0.23f, 0.25f, 0.20f, 0.13f};

            for (int i = 0; i < 4; ++i)
            {
                groundFogShader.setFloat("opacity", fogOpacities[i]);
                groundFogShader.setFloat("layerIndex", (float)i);

                groundFogShader.setMat4("model",
                                        makeModel({-1.8f, fogHeights[i], -24.0f},
                                                  {48.0f, 1.0f, 38.0f}));
                plane.draw();
            }

            glDepthMask(depthWriteWasEnabled);
            if (cullWasEnabled)
                glEnable(GL_CULL_FACE);
            if (!blendWasEnabled)
                glDisable(GL_BLEND);
            shader.use();
        }

        {
            GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
            GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
            GLboolean depthWriteWasEnabled;
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteWasEnabled);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);

            moonBeamShader.use();
            moonBeamShader.setMat4("projection", glm::perspective(
                                                     glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1),
                                                     0.1f, 100.0f));
            moonBeamShader.setMat4("view", camera.getViewMatrix());
            moonBeamShader.setMat4("model", glm::mat4(1.0f));
            moonBeamShader.setVec2("uvScale", glm::vec2(1.0f));
            moonBeamShader.setFloat("time", now);
            moonBeamShader.setFloat("beamStrength", 0.08f);
            moonBeams.draw();

            glDepthMask(depthWriteWasEnabled);
            if (cullWasEnabled)
                glEnable(GL_CULL_FACE);
            if (!blendWasEnabled)
                glDisable(GL_BLEND);
            shader.use();
        }

        {
            const glm::vec3 crystalCenter(0.83f, 1.285f + 0.170f, 4.60f);
            glm::vec3 toCamera = camera.position - crystalCenter;
            if (glm::length(toCamera) > 0.001f)
                toCamera = glm::normalize(toCamera);
            else
                toCamera = glm::vec3(0.0f, 0.0f, -1.0f);

            glm::mat4 glowModel = glm::inverse(camera.getViewMatrix());
            glowModel[3] = glm::vec4(crystalCenter - toCamera * 0.075f, 1.0f);
            glowModel = glm::rotate(glowModel, glm::radians(90.0f),
                                    glm::vec3(1.0f, 0.0f, 0.0f));
            glowModel = glm::scale(glowModel, glm::vec3(0.70f, 1.0f, 0.70f));

            const GLboolean blendBefore = glIsEnabled(GL_BLEND);
            const GLboolean cullBefore = glIsEnabled(GL_CULL_FACE);
            GLboolean depthWriteBefore = GL_TRUE;
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteBefore);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);

            crystalGlowShader.use();
            crystalGlowShader.setMat4("projection", glm::perspective(
                                                        glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1),
                                                        0.1f, 100.0f));
            crystalGlowShader.setMat4("view", camera.getViewMatrix());
            crystalGlowShader.setMat4("model", glowModel);
            crystalGlowShader.setFloat("glowStrength",
                                       0.68f * crystalLightPulse);
            plane.draw();

            glDepthMask(depthWriteBefore);
            if (cullBefore)
                glEnable(GL_CULL_FACE);
            if (!blendBefore)
                glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            shader.use();
        }

        {
            hud.begin(w, h);
            float cx = w / 2.0f, cy = h / 2.0f;
            const glm::vec3 WHITE(1), GREY(0.6f), GREEN(0.55f, 1.0f, 0.55f), ORANGE(1.0f, 0.65f, 0.15f);

            bool done[7] = {
                candlesLit() == 3, candyCount() == 3, potionDone, keyGot,
                tombCount() == 3, ghostCount() == 3, escaped};
            int cur = 0;
            while (cur < 7 && done[cur])
                cur++;
            std::string lines[7] = {
                std::string(done[0] ? "[x] " : "[ ] ") + "Acender as velas  " + std::to_string(candlesLit()) + "/3",
                std::string(done[1] ? "[x] " : "[ ] ") + "Achar os doces    " + std::to_string(candyCount()) + "/3",
                std::string(done[2] ? "[x] " : "[ ] ") + "Ritual no caldeirao",
                std::string(done[3] ? "[x] " : "[ ] ") + "Pegar a chave",
                std::string(done[4] ? "[x] " : "[ ] ") + "Examinar lapides " + std::to_string(tombCount()) + "/3",
                std::string(done[5] ? "[x] " : "[ ] ") + "Encontrar espiritos " + std::to_string(ghostCount()) + "/3",
                std::string(done[6] ? "[x] " : "[ ] ") + "Fugir pela porta"};
            hud.rect(12, 12, 360, 202, glm::vec4(0, 0, 0, 0.55f));
            hud.text(24, 22, "MISSAO: QUEBRE A MALDICAO", 2.0f, ORANGE);
            for (int i = 0; i < 7; i++)
                hud.text(24, 52 + i * 22, lines[i], 2.0f, done[i] ? GREEN : (i == cur ? WHITE : GREY));

            double t = (escaped ? escapeTime : glfwGetTime()) - startTime;
            char tb[16];
            std::snprintf(tb, sizeof(tb), "%02d:%02d", (int)t / 60, (int)t % 60);
            hud.rect(w - 112, 12, 100, 34, glm::vec4(0, 0, 0, 0.55f));
            hud.text(w - 100, 20, tb, 3.0f, WHITE);

            hud.text(cx - 5, cy - 8, "+", 2.0f, aimed >= 0 ? ORANGE : glm::vec3(1, 1, 1));
            if (aimed >= 0)
                hud.textCentered(cx, cy + 24, promptFor(aimed), 2.5f, ORANGE);

            if (glfwGetTime() < toastUntil)
            {
                float tw = hud.width(toast, 2.5f);
                hud.rect(cx - tw / 2 - 14, h - 84, tw + 28, 40, glm::vec4(0, 0, 0, 0.6f));
                hud.textCentered(cx, h - 74, toast, 2.5f, WHITE);
            }

            if (escaped && doorTarget)
            {
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
