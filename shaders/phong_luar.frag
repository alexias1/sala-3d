#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
out vec4 FragColor;

struct Material {
    vec3 ka;        // reflexao ambiente
    vec3 kd;        // reflexao difusa
    vec3 ks;        // reflexao especular
    float shininess;
    vec3 emissive;  // luz propria (lampada, TV, ceu)
};

struct Light {
    vec3 position;
    vec3 color;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 9
uniform Material material;
uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform vec3 viewPos;
uniform sampler2D tex;
uniform sampler2D normalMap;
uniform float normalStrength;

// Luar direcional: nao ocupa uma das oito luzes de ponto.
uniform vec3 moonLightDirection;
uniform vec3 moonLightColor;
uniform float moonIndoorBoost; // reforco do luar so na sala
uniform float isMountain; // 1.0 = montanha; 0.0 = outros objetos

uniform vec3 fogColor;
uniform float fogDensity;

const vec3 SKY_AMBIENT    = vec3(1.00, 0.80, 1.30);
const vec3 GROUND_AMBIENT = vec3(0.40, 0.35, 0.45);
const float GAMMA = 2.2;

vec3 toLinear(vec3 c) { return pow(c, vec3(GAMMA)); }

// A luz lunar so entra na sala pela abertura da janela.
// Esta e uma aproximacao geometrica, nao shadow mapping.
float moonVisibility(vec3 p, vec3 L) {
    const float WALL_Z = -4.90;
    if (p.z < -5.1) return 1.0; // exterior
    if (L.z >= -0.01) return 0.0;

    float t = (WALL_Z - p.z) / L.z;
    if (t < 0.0) return 0.0;
    vec3 hit = p + L * t;

    // Janela: centro (-1.8, 1.8), largura 2.2, altura 1.4.
    float xMask = smoothstep(-2.90, -2.82, hit.x) *
                  (1.0 - smoothstep(-0.78, -0.70, hit.x));
    float yMask = smoothstep(1.10, 1.18, hit.y) *
                  (1.0 - smoothstep(2.42, 2.50, hit.y));

    // Sombra aproximada da cruz de madeira.
    float verticalBar = smoothstep(0.028, 0.073, abs(hit.x + 1.8));
    float horizontalBar = smoothstep(0.028, 0.073, abs(hit.y - 1.8));
    return xMask * yMask * verticalBar * horizontalBar;
}

// Base TBN deduzida das derivadas, sem tangentes pre-calculadas.
mat3 cotangentFrame(vec3 N, vec3 p, vec2 uv) {
    vec3 dp1 = dFdx(p), dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv), duv2 = dFdy(uv);
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

void main() {
    vec3 N = normalize(Normal);
    if (normalStrength > 0.0) {
        vec3 n = texture(normalMap, UV).rgb * 2.0 - 1.0;
        n.xy *= normalStrength;
        N = normalize(cotangentFrame(N, FragPos, UV) * n);
    }
    vec3 V = normalize(viewPos - FragPos);

    // A textura sRGB ja chega em espaco linear do hardware.
    vec3 texColor = texture(tex, UV).rgb;
    vec3 ka = toLinear(material.ka);
    vec3 kd = toLinear(material.kd);
    vec3 ks = toLinear(material.ks);
    vec3 emissive = toLinear(material.emissive);

    // Iluminacao ambiente hemisferica.
    vec3 ambient = mix(GROUND_AMBIENT, SKY_AMBIENT, N.y * 0.5 + 0.5);
    vec3 result = ka * texColor * ambient;

    // Montanhas recebem apenas 10% da iluminacao ambiente.
    if (isMountain > 0.5) {
        result *= 0.10;
    }

    // Oito luzes do cenario nao iluminam as montanhas distantes.
    if (isMountain < 0.5) {
        for (int i = 0; i < numLights; i++) {
            vec3 L = normalize(lights[i].position - FragPos);
            float dist = length(lights[i].position - FragPos);
            float atten = 1.0 / (
                1.0 + lights[i].linear * dist +
                lights[i].quadratic * dist * dist
            );

            float diff = max(dot(N, L), 0.0);
            vec3 H = normalize(L + V);
            float spec = pow(max(dot(N, H), 0.0), material.shininess);
            if (diff <= 0.0) spec = 0.0;

            result += atten * lights[i].color *
                      (kd * texColor * diff + ks * spec);
        }
    }

    // =========================================================
    // ILUMINACAO LUNAR
    // =========================================================
    vec3 moonDir = normalize(moonLightDirection);

    // Montanhas recebem luar em todo o anel de 360 graus.
    // Os demais objetos respeitam a abertura da janela.
    float moonMask = (isMountain > 0.5)
        ? 1.0
        : moonVisibility(FragPos, moonDir);

    float moonDiffuse = max(dot(N, moonDir), 0.0);

    // Luar normal para a grama e para a sala.
    float fill = (FragPos.z < -5.1) ? 0.16 : 0.0;
    vec3 normalMoon = moonLightColor * kd * texColor *
                      (fill + 0.85 * moonDiffuse);

    // Gradiente de luar concentrado nas partes altas das montanhas.
    float peakGradient = smoothstep(1.0, 4.8, FragPos.y);
    peakGradient = pow(peakGradient, 1.5);

    // 0.002 = base quase escura; 0.012 = pico suavemente iluminado.
    float peakIntensity = mix(0.002, 0.012, peakGradient);
    vec3 mountainMoon = moonLightColor * texColor * peakIntensity;

    vec3 moonContribution = mix(normalMoon, mountainMoon,
                                clamp(isMountain, 0.0, 1.0));

    // Aplicada uma unica vez, depois de calcular moonMask e moonContribution.
    // So aumenta a luz que atravessou a janela, sem alterar a parte externa.
    float indoorFactor = (FragPos.z > -4.90) ? moonIndoorBoost : 1.0;
    result += moonMask * moonContribution * indoorFactor;

    // Emissao propria somente para objetos comuns, incluindo o ceu.
    // As montanhas nao somam a emissao do mountainMat.
    if (isMountain < 0.5) {
        result += emissive;
    }

    // Neblina de distancia original do shader Phong (nao e a fog rasteira).
    float d = length(viewPos - FragPos);
    float fog = 1.0 - exp(-pow(d * fogDensity, 2.0));
    result = mix(result, toLinear(fogColor), clamp(fog, 0.0, 1.0));

    result = pow(clamp(result, 0.0, 1.0), vec3(1.0 / GAMMA));
    FragColor = vec4(result, 1.0);
}
