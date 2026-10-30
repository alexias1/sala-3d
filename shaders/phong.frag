#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
out vec4 FragColor;

struct Material {
    vec3 ka;        // reflexão ambiente
    vec3 kd;        // reflexão difusa
    vec3 ks;        // reflexão especular
    float shininess;
    vec3 emissive;  // luz própria (lâmpada, TV)
};

struct Light {
    vec3 position;
    vec3 color;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 8
uniform Material material;
uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform vec3 viewPos;
uniform sampler2D tex;

// Neblina (cor "como aparece na tela"; o shader converte para linear)
uniform vec3 fogColor;
uniform float fogDensity;

// Ambiente hemisférico: tom do "céu" (normais para cima) e do "chão" (para baixo)
const vec3 SKY_AMBIENT    = vec3(1.00, 0.80, 1.30);   // roxo-azulado
const vec3 GROUND_AMBIENT = vec3(0.40, 0.35, 0.45);   // quase escuro

const float GAMMA = 2.2;

// As cores dos materiais foram "pintadas" olhando a tela (sRGB), então as
// convertemos para linear antes de calcular a luz.
vec3 toLinear(vec3 c) { return pow(c, vec3(GAMMA)); }

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // A textura é carregada como sRGB, então o hardware já entrega o valor em linear
    vec3 texColor = texture(tex, UV).rgb;

    vec3 ka = toLinear(material.ka);
    vec3 kd = toLinear(material.kd);
    vec3 ks = toLinear(material.ks);
    vec3 emissive = toLinear(material.emissive);

    // ambiente hemisférico
    vec3 ambient = mix(GROUND_AMBIENT, SKY_AMBIENT, N.y * 0.5 + 0.5);
    vec3 result = ka * texColor * ambient;

    for (int i = 0; i < numLights; i++) {
        vec3 L = normalize(lights[i].position - FragPos);
        float dist = length(lights[i].position - FragPos);
        float atten = 1.0 / (1.0 + lights[i].linear * dist + lights[i].quadratic * dist * dist);

        float diff = max(dot(N, L), 0.0);                 // difusa
        vec3 H = normalize(L + V);                        // vetor "meio" (Blinn)
        float spec = pow(max(dot(N, H), 0.0), material.shininess);
        if (diff <= 0.0) spec = 0.0;

        result += atten * lights[i].color * (kd * texColor * diff + ks * spec);
    }

    result += emissive;

    // neblina exponencial ao quadrado (suave perto, densa longe)
    float d = length(viewPos - FragPos);
    float fog = 1.0 - exp(-pow(d * fogDensity, 2.0));
    result = mix(result, toLinear(fogColor), clamp(fog, 0.0, 1.0));

    // correção de gama: volta de linear para a tela
    result = pow(clamp(result, 0.0, 1.0), vec3(1.0 / GAMMA));
    FragColor = vec4(result, 1.0);
}
