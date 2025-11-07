#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 colour;

const int MAX_POINT_LIGHTS = 3;

struct Light {
    vec3  colour;
    float ambientIntensity;
    float diffuseIntensity;
};

struct DirectionalLight {
    Light base;
    vec3  direction;
};

struct PointLight {
    Light base;
    vec3  position;
    float constant;
    float linear;
    float exponent;
};

struct Material {
    float specularIntensity;
    float shininess;
};

uniform int pointLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight      pointLights[MAX_POINT_LIGHTS];

uniform sampler2D theTexture;
uniform Material  material;
uniform vec3      eyePosition;

vec4 CalcLightByDirection(Light light, vec3 L)
{
    vec3 N = normalize(Normal);
    vec3 Ln = normalize(-L);          // к фрагменту
    vec4 ambient  = vec4(light.colour * light.ambientIntensity, 1.0);

    float NdotL   = max(dot(N, Ln), 0.0);
    vec4 diffuse  = vec4(light.colour * light.diffuseIntensity * NdotL, 1.0);

    vec4 specular = vec4(0.0);
    if (NdotL > 0.0) {
        vec3 V = normalize(eyePosition - FragPos);
        vec3 R = reflect(-Ln, N);
        float s = pow(max(dot(V, R), 0.0), material.shininess);
        specular = vec4(light.colour * material.specularIntensity * s, 1.0);
    }
    return ambient + diffuse + specular;
}

vec4 CalcDirectionalLight()
{
    // направление из источника к фрагменту — берём -dir
    return CalcLightByDirection(directionalLight.base, -directionalLight.direction);
}

vec4 CalcPointLights()
{
    vec4 total = vec4(0.0);
    for (int i = 0; i < pointLightCount; ++i) {
        vec3 L = FragPos - pointLights[i].position; // из источника к фрагменту
        float d = length(L);
        vec4 c  = CalcLightByDirection(pointLights[i].base, L);
        float att = pointLights[i].exponent * d * d +
        pointLights[i].linear   * d +
        pointLights[i].constant;
        total += c / att;
    }
    return total;
}

void main()
{
    vec4 lit = CalcDirectionalLight() + CalcPointLights();
    colour = texture(theTexture, TexCoord) * lit;
}
