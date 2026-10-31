#version 330 core
layout (location = 0) in vec2 aPos;   // pixels, y para baixo

uniform vec2 screen;   // tamanho da janela em pixels
uniform vec2 offset;   // posição do texto na tela
uniform float scale;   // tamanho do texto

void main() {
    vec2 p = aPos * scale + offset;
    gl_Position = vec4(p.x / screen.x * 2.0 - 1.0, 1.0 - p.y / screen.y * 2.0, 0.0, 1.0);
}
