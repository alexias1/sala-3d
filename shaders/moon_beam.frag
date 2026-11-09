#version 330 core
// Feixes de luar translucidos com poeira animada
// ---------- Entradas / saida ----------
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
out vec4 FragColor;

// ---------- Uniforms ----------
uniform float time;
uniform float beamStrength;

// Ruido procedural barato para criar particulas de poeira e luz espalhada.
float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

// Ruido suave interpolado
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

// Ruido fractal (3 oitavas)
float fbm(vec2 p)
{
    float s = 0.0;
    float w = 0.55;
    for (int i = 0; i < 3; ++i)
    {
        s += w * noise2d(p);
        p = p * 2.04 + 12.5;
        w *= 0.48;
    }
    return s;
}

// ---------- Main ----------
void main()
{
    // Fita dissolve nas bordas, no comeco e no final do feixe.
    float edge = pow(max(0.0, 1.0 - abs(UV.x * 2.0 - 1.0)), 1.35);
    float front = smoothstep(0.0, 0.10, UV.y);
    float back  = 1.0 - smoothstep(0.53, 0.98, UV.y);

    // Movimento minimo: nao parece uma lampada girando.
    vec2 drift = vec2(time * 0.045, -time * 0.02);
    float cloud = fbm(FragPos.xz * 0.47 + FragPos.y * 0.20 + drift);
    float detail = fbm(FragPos.xz * 1.25 - drift * 0.55);
    float uneven = smoothstep(0.16, 0.73, cloud * 0.78 + detail * 0.22);

    float alpha = beamStrength * edge * front * back *
                  (0.30 + 0.70 * uneven);

    // Um luar azul-prateado, sem adicionar luz nas montanhas nem na fog.
    vec3 color = vec3(0.54, 0.66, 0.86);
    FragColor = vec4(color, clamp(alpha, 0.0, 0.34));
}
