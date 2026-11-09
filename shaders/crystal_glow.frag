
#version 330 core

// Halo roxo radial, transparente nas bordas
in vec2 UV;
out vec4 FragColor;

uniform float glowStrength;

void main()
{
    // Distancia do centro da imagem
    float dist = length(UV - vec2(0.5)) * 2.0;

    // Brilho suave que desaparece nas bordas
    float glow = 1.0 - smoothstep(0.05, 1.0, dist);

    glow = pow(glow, 1.8);

    // Roxo magico
    vec3 glowColor = vec3(0.55, 0.25, 1.0);

    FragColor = vec4(glowColor, glow * glowStrength);
}
