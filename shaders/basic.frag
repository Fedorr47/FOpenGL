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

struct SpotLight {
    PointLight base;
    vec3 direction;
    float edge;
};

struct Material {
    float specularIntensity;
    float shininess;
};

const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS = 8;

uniform DirectionalLight directionalLight;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int pointLightCount;

uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int spotLightCount;

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

vec3 CalcPointLight(PointLight pLight)
{
    vec3 V = normalize(eyePosition - FragPos);
    vec3 N = normalize(Normal);

    vec3 L = pLight.position - FragPos;
    float d = length(L);
    L = L / max(d, 1e-6);

    vec3 c = ApplyLight(pLight.base, L, N, V);
    float atten = pLight.constant + pLight.linear * d + pLight.exponent * d * d;

    return c / max(atten, 1e-4);
}

vec4 CalcPointLights()
{
    vec3 total = vec3(0.0);
    for (int i = 0; i < pointLightCount; ++i) {
        total += CalcPointLight(pointLights[i]);
    }
    return vec4(total, 1.0);
}

vec3 CalcSpotLight(SpotLight sLight)
{
    vec3 L = normalize(FragPos - sLight.base.position);
    vec3 D = normalize(sLight.direction);
    
    float sl = dot(D, L);
    
    if (sl <= sLight.edge)
    return vec3(0.0);


    // 1 - (1 - sl) / (1 - edge)
    float denom = max(1.0 - sLight.edge, 1e-6);
    float spotFactor = clamp((sl - sLight.edge) / denom, 0.0, 1.0);
    
    vec3 c = CalcPointLight(sLight.base);
    
    return c * spotFactor;
}

vec4 CalcSpotLights()
{
    vec3 total = vec3(0.0);
    for (int i = 0; i < spotLightCount; ++i) {
        total += CalcSpotLight(spotLights[i]);
    }
    return vec4(total, 1.0);
}

void main()
{
    vec3 albedo = texture(theTexture, TexCoord).rgb;

    vec4 dirCol   = CalcDirectional();
    vec4 pointsCol = CalcPointLights();
    vec4 spotsCol = CalcSpotLights();
    vec3 lit = dirCol.rgb + pointsCol.rgb;
    
    colour = vec4(albedo * (dirCol.rgb + pointsCol.rgb + spotsCol.rgb), 1.0);
}
