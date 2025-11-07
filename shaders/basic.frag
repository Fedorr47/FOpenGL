#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 colour;

struct Light {
    vec3 colour;
    float ambientIntensity;
    float diffuseIntensity;
};

struct DirectionalLight {
    Light base;
    vec3 direction;
};

struct PointLight {
    Light base;
    vec3 position;
    float constant;
    float linear;
    float exponent;
};

struct Material {
    float specularIntensity;
    float shininess;
};

const int MAX_POINT_LIGHTS = 8;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int pointLightCount;

uniform Material material;
uniform vec3 eyePosition;
uniform sampler2D theTexture;

vec3 ApplyLight(Light light, vec3 L, vec3 N, vec3 V)
{
    L = normalize(L); N = normalize(N); V = normalize(V);
    vec3 ambient = light.colour * light.ambientIntensity;
    float NdotL = max(dot(N,L), 0.0);
    vec3 diffuse = light.colour * light.diffuseIntensity * NdotL;

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), material.shininess);
    vec3 specular = light.colour * material.specularIntensity * spec;

    return ambient + diffuse + specular;
}

vec4 CalcDirectional()
{
    vec3 L = -directionalLight.direction;
    vec3 V = eyePosition - FragPos;
    vec3 N = Normal;
    vec3 res = ApplyLight(directionalLight.base, L, N, V);
    return vec4(res, 1.0);
}

vec4 CalcPoints()
{
    vec3 V = eyePosition - FragPos;
    vec3 N = Normal;
    vec3 total = vec3(0.0);
    for (int i=0;i<pointLightCount;i++) {
        vec3 L = pointLights[i].position - FragPos;
        float d = length(L);
        vec3 c = ApplyLight(pointLights[i].base, L, N, V);
        float atten = pointLights[i].constant + pointLights[i].linear * d + pointLights[i].exponent * d * d;
        total += c / max(atten, 0.0001);
    }
    return vec4(total, 1.0);
}

void main()
{
    vec3 albedo = texture(theTexture, TexCoord).rgb;

    vec4 dirCol   = CalcDirectional();
    vec4 pointsCol= CalcPoints();
    vec3 lit = dirCol.rgb + pointsCol.rgb;
    
    colour = vec4(albedo * (dirCol.rgb + pointsCol.rgb), 1.0);
}
