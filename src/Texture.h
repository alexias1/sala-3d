#pragma once
#include <glad/glad.h>
#include <string>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

inline void setTexParams() {
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// Textura branca 1x1: objetos sem textura usam esta (cor = só o material)
inline unsigned int makeWhiteTexture() {
    unsigned int id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    unsigned char white[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    setTexParams();
    return id;
}

inline unsigned int loadTexture(const std::string& path) {
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);   // imagem tem y para baixo, OpenGL para cima
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);  // força RGBA
    if (!data) {
        std::cerr << "Nao consegui carregar textura: " << path << "\n";
        return makeWhiteTexture();             // não quebra o programa
    }
    unsigned int id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    setTexParams();
    stbi_image_free(data);
    return id;
}