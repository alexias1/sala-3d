#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Shader.h"
#include "Camera.h"

const int SCR_W = 1280, SCR_H = 720;

Camera camera;
float lastX = SCR_W / 2.0f, lastY = SCR_H / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos;   // invertido: y da tela cresce pra baixo
    lastX = (float)xpos; lastY = (float)ypos;
    camera.rotate(dx, dy);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.moveForward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.moveBackward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.moveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.moveRight(deltaTime);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // prende e esconde o mouse

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "Falha ao iniciar GLAD\n"; return -1; }
    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/basic.vert", "shaders/basic.frag");

    // Cubo: 24 vértices (4 por face), x y z  r g b
    float v[] = {
        // frente (z+) vermelho
        -0.5f,-0.5f, 0.5f, 1,0,0,   0.5f,-0.5f, 0.5f, 1,0,0,   0.5f, 0.5f, 0.5f, 1,0,0,  -0.5f, 0.5f, 0.5f, 1,0,0,
        // trás (z-) verde
         0.5f,-0.5f,-0.5f, 0,1,0,  -0.5f,-0.5f,-0.5f, 0,1,0,  -0.5f, 0.5f,-0.5f, 0,1,0,   0.5f, 0.5f,-0.5f, 0,1,0,
        // esquerda (x-) azul
        -0.5f,-0.5f,-0.5f, 0,0,1,  -0.5f,-0.5f, 0.5f, 0,0,1,  -0.5f, 0.5f, 0.5f, 0,0,1,  -0.5f, 0.5f,-0.5f, 0,0,1,
        // direita (x+) amarelo
         0.5f,-0.5f, 0.5f, 1,1,0,   0.5f,-0.5f,-0.5f, 1,1,0,   0.5f, 0.5f,-0.5f, 1,1,0,   0.5f, 0.5f, 0.5f, 1,1,0,
        // topo (y+) ciano
        -0.5f, 0.5f, 0.5f, 0,1,1,   0.5f, 0.5f, 0.5f, 0,1,1,   0.5f, 0.5f,-0.5f, 0,1,1,  -0.5f, 0.5f,-0.5f, 0,1,1,
        // base (y-) magenta
        -0.5f,-0.5f,-0.5f, 1,0,1,   0.5f,-0.5f,-0.5f, 1,0,1,   0.5f,-0.5f, 0.5f, 1,0,1,  -0.5f,-0.5f, 0.5f, 1,0,1
    };
    // Índices: cada face são 2 triângulos (4 vértices reaproveitados)
    unsigned int idx[36];
    for (int f = 0; f < 6; f++) {
        unsigned int b = f * 4, o = f * 6;
        idx[o]=b; idx[o+1]=b+1; idx[o+2]=b+2;
        idx[o+3]=b; idx[o+4]=b+2; idx[o+5]=b+3;
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;   // tempo do quadro: movimento igual em qualquer PC
        lastFrame = now;

        processInput(window);

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)w / (float)(h > 0 ? h : 1), 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", camera.getViewMatrix());

        glBindVertexArray(VAO);

        // Cubo 1: gira no lugar (rotação)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, now, glm::vec3(0.3f, 1.0f, 0.0f));
        shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Cubo 2: esticado como um "chão" (escala + translação)
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 0.2f, 10.0f));
        shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}