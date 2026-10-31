#pragma once
#include <glad/glad.h>
#include <string>
#include <iostream>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Constantes da extensão EXT_texture_filter_anisotropic (o glad gerado não as inclui)
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

inline void setTexParams() {
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Filtro anisotrópico: texturas em ângulo rasante (chão ao longe) ficam nítidas.
    // Se a placa não suportar, o erro é descartado e nada acontece.
    while (glGetError() != GL_NO_ERROR) {}
    float maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    if (glGetError() == GL_NO_ERROR && maxAniso > 1.0f)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(8.0f, maxAniso));
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
    // GL_SRGB8_ALPHA8: a GPU converte sRGB -> linear ao amostrar (necessário para a correção de gama)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    setTexParams();
    stbi_image_free(data);
    return id;
}

// Mapa de normais: dados (não cor!), então vão em formato LINEAR (GL_RGBA8), sem conversão sRGB.
// Retorna 0 se o arquivo não existir (o programa então desenha sem relevo).
inline unsigned int loadNormalMap(const std::string& path) {
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
    if (!data) {
        std::cerr << "Nao consegui carregar mapa de normais: " << path << "\n";
        return 0;
    }
    unsigned int id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    setTexParams();
    stbi_image_free(data);
    return id;
}
