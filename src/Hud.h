#pragma once
// HUD 2D: texto e retângulos na tela, SEM biblioteca de fontes.
// Usa stb_easy_font.h, que transforma cada letra em quads (retângulos).
// Limitação: só ASCII (sem acentos nem cedilha).
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"
#include "stb_easy_font.h"

class Hud {
public:
    Hud() : shader("shaders/hud.vert", "shaders/hud.frag") {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(buf), nullptr, GL_DYNAMIC_DRAW);
        // stb_easy_font gera 4 vértices (16 bytes cada: x,y,z + cor) por letra-retângulo.
        // Só usamos x,y (2 floats), com passo (stride) de 16 bytes.
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);
        glEnableVertexAttribArray(0);
        // OpenGL 3.3 não tem GL_QUADS: cada quad vira 2 triângulos via índices
        std::vector<unsigned int> idx;
        for (unsigned int q = 0; q < MAXQ; q++) {
            unsigned int b = q * 4;
            idx.insert(idx.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    }

    void begin(int w, int h) {
        sw = (float)w; sh = (float)h;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader.use();
        shader.setVec2("screen", glm::vec2(sw, sh));
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void end() {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    // Retângulo em pixels (origem no canto superior esquerdo)
    void rect(float x, float y, float w, float h, glm::vec4 color) {
        float v[16] = {x, y, 0, 0,  x + w, y, 0, 0,  x + w, y + h, 0, 0,  x, y + h, 0, 0};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
        shader.setVec2("offset", glm::vec2(0));
        shader.setFloat("scale", 1.0f);
        shader.setVec4("color", color);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    float width(const std::string& s, float scale) {
        return stb_easy_font_width((char*)s.c_str()) * scale;
    }

    // Texto com sombra preta de 1-2 px (melhora a leitura sobre qualquer fundo)
    void text(float x, float y, const std::string& s, float scale, glm::vec3 color) {
        int quads = stb_easy_font_print(0, 0, (char*)s.c_str(), nullptr, buf, sizeof(buf));
        if (quads <= 0) return;
        glBufferSubData(GL_ARRAY_BUFFER, 0, quads * 64, buf);
        shader.setFloat("scale", scale);
        shader.setVec2("offset", glm::vec2(x + scale, y + scale));
        shader.setVec4("color", glm::vec4(0, 0, 0, 0.9f));
        glDrawElements(GL_TRIANGLES, quads * 6, GL_UNSIGNED_INT, 0);
        shader.setVec2("offset", glm::vec2(x, y));
        shader.setVec4("color", glm::vec4(color, 1.0f));
        glDrawElements(GL_TRIANGLES, quads * 6, GL_UNSIGNED_INT, 0);
    }

    void textCentered(float cx, float y, const std::string& s, float scale, glm::vec3 color) {
        text(cx - width(s, scale) / 2.0f, y, s, scale, color);
    }

private:
    static const int MAXQ = 1024;
    Shader shader;
    unsigned int vao = 0, vbo = 0, ebo = 0;
    float sw = 1, sh = 1;
    char buf[MAXQ * 64];
};
