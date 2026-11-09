#version 330 core
// Lapides externas: frente escura, luar no topo e contraluz nas bordas
// ---------- Entradas / saida ----------
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
out vec4 FragColor;

// ---------- Material e uniforms ----------
struct Material {
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float shininess;
    vec3 emissive;
};

uniform Material material;
uniform vec3 viewPos;
uniform vec3 moonLightDirection;
uniform vec3 moonLightColor;
uniform sampler2D tex;

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(moonLightDirection); // da lapide em direcao a lua

    vec3 stone = texture(tex, UV).rgb * material.kd;

    // ---------------------------------------------------------
    // BASE: frente bem escura
    // ---------------------------------------------------------
    vec3 shade = stone * 0.015;

    // ---------------------------------------------------------
    // LUZ SÓ NAS PARTES DE CIMA
    // - favorece o topo arredondado
    // - evita clarear a face frontal inteira
    // ---------------------------------------------------------
    float moonDiffuse = max(dot(N, L), 0.0);
    float topMask = pow(max(N.y, 0.0), 1.4);
    shade += stone * moonDiffuse * topMask * 0.22;

    // ---------------------------------------------------------
    // CONTRALUZ NAS BORDAS
    // - contorno suave
    // - mais forte quando a face esta contra a lua
    // ---------------------------------------------------------
    float rim = pow(1.0 - max(dot(N, V), 0.0), 3.4);
    float backMask = pow(max(-dot(N, L), 0.0), 1.2);
    vec3 moonTint = normalize(moonLightColor + vec3(0.0001));

    shade += moonTint * 0.10 * rim * (0.35 + 0.65 * backMask);

    // ---------------------------------------------------------
    // LEVE REFORCO NO TOPO
    // ---------------------------------------------------------
    shade += moonTint * 0.035 * topMask;

    // gamma
    shade = pow(clamp(shade, 0.0, 1.0), vec3(1.0 / 2.2));
    FragColor = vec4(shade, 1.0);
}