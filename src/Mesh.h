#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <cstddef>

struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };

class Mesh {
public:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;

    Mesh(const std::vector<Vertex>& v, const std::vector<unsigned int>& idx) {
        indexCount = (int)idx.size();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(2);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    // ---------- Primitivas (todas centradas na origem, tamanho 1) ----------

    // Cubo: 6 faces, cada uma com sua normal
    static Mesh cube() {
        std::vector<Vertex> v; std::vector<unsigned int> idx;
        // normal, eixo u, eixo v (cross(u,v) = normal)
        glm::vec3 faces[6][3] = {
            {{0,0,1},  {1,0,0},  {0,1,0}},
            {{0,0,-1}, {-1,0,0}, {0,1,0}},
            {{1,0,0},  {0,0,-1}, {0,1,0}},
            {{-1,0,0}, {0,0,1},  {0,1,0}},
            {{0,1,0},  {1,0,0},  {0,0,-1}},
            {{0,-1,0}, {1,0,0},  {0,0,1}}
        };
        for (int f = 0; f < 6; f++) {
            glm::vec3 n = faces[f][0], u = faces[f][1], w = faces[f][2];
            glm::vec3 c = n * 0.5f;
            unsigned int b = (unsigned int)v.size();
            v.push_back({c + (-u - w) * 0.5f, n, {0, 0}});
            v.push_back({c + ( u - w) * 0.5f, n, {1, 0}});
            v.push_back({c + ( u + w) * 0.5f, n, {1, 1}});
            v.push_back({c + (-u + w) * 0.5f, n, {0, 1}});
            idx.insert(idx.end(), {b, b+1, b+2, b, b+2, b+3});
        }
        return Mesh(v, idx);
    }

    // Plano horizontal (XZ), normal para cima
    static Mesh plane() {
        std::vector<Vertex> v = {
            {{-0.5f,0, 0.5f}, {0,1,0}, {0,0}},
            {{ 0.5f,0, 0.5f}, {0,1,0}, {1,0}},
            {{ 0.5f,0,-0.5f}, {0,1,0}, {1,1}},
            {{-0.5f,0,-0.5f}, {0,1,0}, {0,1}}
        };
        return Mesh(v, {0,1,2, 0,2,3});
    }

    // Cilindro: raio 0.5, altura 1 (de y=-0.5 a y=0.5)
    static Mesh cylinder(int seg = 32) {
        std::vector<Vertex> v; std::vector<unsigned int> idx;
        const float PI = 3.14159265f;
        // lateral
        for (int i = 0; i <= seg; i++) {
            float a = 2 * PI * i / seg;
            float x = std::cos(a), z = std::sin(a);
            glm::vec3 n(x, 0, z);
            v.push_back({{0.5f * x, -0.5f, 0.5f * z}, n, {(float)i / seg, 0}});
            v.push_back({{0.5f * x,  0.5f, 0.5f * z}, n, {(float)i / seg, 1}});
        }
        for (int i = 0; i < seg; i++) {
            unsigned int b = i * 2;
            idx.insert(idx.end(), {b, b+1, b+2, b+1, b+3, b+2});
        }
        // tampas (topo e base)
        for (int cap = 0; cap < 2; cap++) {
            float y = cap ? 0.5f : -0.5f;
            glm::vec3 n(0, cap ? 1.0f : -1.0f, 0);
            unsigned int center = (unsigned int)v.size();
            v.push_back({{0, y, 0}, n, {0.5f, 0.5f}});
            for (int i = 0; i <= seg; i++) {
                float a = 2 * PI * i / seg;
                v.push_back({{0.5f * std::cos(a), y, 0.5f * std::sin(a)}, n,
                             {0.5f + 0.5f * std::cos(a), 0.5f + 0.5f * std::sin(a)}});
            }
            for (int i = 0; i < seg; i++)
                idx.insert(idx.end(), {center, center + 1 + i, center + 2 + i});
        }
        return Mesh(v, idx);
    }

    // Esfera: raio 0.5
    static Mesh sphere(int stacks = 24, int slices = 32) {
        std::vector<Vertex> v; std::vector<unsigned int> idx;
        const float PI = 3.14159265f;
        for (int i = 0; i <= stacks; i++) {
            float phi = PI * i / stacks;
            for (int j = 0; j <= slices; j++) {
                float th = 2 * PI * j / slices;
                glm::vec3 n(std::sin(phi) * std::cos(th), std::cos(phi), std::sin(phi) * std::sin(th));
                v.push_back({n * 0.5f, n, {(float)j / slices, 1.0f - (float)i / stacks}});
            }
        }
        for (int i = 0; i < stacks; i++)
            for (int j = 0; j < slices; j++) {
                unsigned int a = i * (slices + 1) + j, b = a + slices + 1;
                idx.insert(idx.end(), {a, b, a+1, a+1, b, b+1});
            }
        return Mesh(v, idx);
    }
};