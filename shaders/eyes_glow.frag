#version 330 core
// Dois pequenos olhos ambar. Plano transparente com centro e halo suaves.
// Usado junto de phong.vert, que fornece as coordenadas UV do Mesh::plane().
in vec2 UV;
out vec4 FragColor;

// ---------- Uniforms ----------
uniform float visibility;

void main()
{
    vec2 p = (UV - vec2(0.5)) * 2.0;
    float r2 = dot(p, p);
    float r = sqrt(r2);

    // Pequeno ponto brilhante e halo suave, sem borda quadrada.
    float core = exp(-r2 * 95.0);
    float halo = exp(-r2 * 9.0) * (1.0 - smoothstep(0.70, 1.0, r));
    float intensity = (0.78 * core + 0.18 * halo) * visibility;

    vec3 amber = mix(vec3(0.85, 0.42, 0.05),
                     vec3(1.00, 0.90, 0.33), core);
    FragColor = vec4(amber, clamp(intensity, 0.0, 1.0));
}
