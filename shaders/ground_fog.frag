#version 330 core

// Neblina baixa e animada, com contorno irregular, sem textura.
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

// ---------- Uniforms ----------
uniform float time;
uniform float opacity;
uniform float layerIndex;

// ---------- Ruido procedural ----------
// Hash pseudoaleatorio 2D
float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

// Ruido suave interpolado.
float noise2d(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// Nuvens com detalhes em varias escalas (fractal Brownian motion).
float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 4; ++i)
    {
        value += amplitude * noise2d(p);
        p = p * 2.03 + 17.3;
        amplitude *= 0.5;
    }
    return value;
}

// ---------- Main ----------
void main()
{
    vec2 pos = FragPos.xz;
    float t = time;

    // Fluxos lentos e diferentes em cada camada.
    vec2 drift = vec2(t * 0.055, -t * 0.025);
    vec2 offset = vec2(layerIndex * 18.7, layerIndex * 9.4);
    float broad = fbm(pos * 0.13 + drift + offset);
    float detail = fbm(pos * 0.31 - drift * 0.75 + offset * 1.6);

    // Manchas macias de densidade irregular: nada de quadrado uniforme.
    float swirls = smoothstep(0.34, 0.66, broad * 0.72 + detail * 0.28);
    float density = 0.22 + 0.78 * swirls;

    // Distancia horizontal ate a parte externa das quatro paredes da casa.
    // Dentro da sala o valor e zero: nenhuma camada de neblina a invade.
    vec2 pastWalls = max(abs(pos) - vec2(5.05), vec2(0.0));
    float outsideDistance = length(pastWalls);
    float fadeNear = smoothstep(0.10, 2.6, outsideDistance);

    // Desaparece radialmente antes das montanhas, sem bordas quadradas.
    float distanceFromHouse = length(pos);
    float fadeFar = 1.0 - smoothstep(19.0, 25.0, distanceFromHouse);

    float alpha = opacity * density * fadeNear * fadeFar;

    // Azul acinzentado, pouco luminoso: neblina fria de Halloween.
    vec3 fogColor = mix(vec3(0.29, 0.34, 0.42),
                        vec3(0.44, 0.48, 0.57), swirls * 0.55);

    FragColor = vec4(fogColor, alpha);
}
