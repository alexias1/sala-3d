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

#define MAX_LIGHTS 4
uniform Material material;
uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform vec3 viewPos;
uniform sampler2D tex;

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 texColor = texture(tex, UV).rgb;

    vec3 result = material.ka * texColor;   // ambiente

    for (int i = 0; i < numLights; i++) {
        vec3 L = normalize(lights[i].position - FragPos);
        float dist = length(lights[i].position - FragPos);
        float atten = 1.0 / (1.0 + lights[i].linear * dist + lights[i].quadratic * dist * dist);

        float diff = max(dot(N, L), 0.0);                 // difusa
        vec3 H = normalize(L + V);                        // vetor "meio" (Blinn)
        float spec = pow(max(dot(N, H), 0.0), material.shininess);
        if (diff <= 0.0) spec = 0.0;

        result += atten * lights[i].color *
                  (material.kd * texColor * diff + material.ks * spec);
    }

    result += material.emissive;
    FragColor = vec4(result, 1.0);
}