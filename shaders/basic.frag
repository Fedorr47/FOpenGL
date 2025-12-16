#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 DirectionalLightSpacePos;
out vec4 colour;

struct Light { vec3 colour; float ambientIntensity; float diffuseIntensity; };
struct DirectionalLight { Light base; vec3 direction; };
struct PointLight { Light base; vec3 position; float constant; float linear; float exponent; };
struct SpotLight { PointLight base; vec3 direction; float edge; };
struct Material { float specularIntensity; float shininess; };

const int MAX_POINT_LIGHTS = 8;
const int MAX_SPOT_LIGHTS  = 8;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int pointLightCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int spotLightCount;

uniform Material material;
uniform vec3 eyePosition;

uniform sampler2D theTexture;
uniform sampler2D directionalShadowMap;

vec3 ApplyLight(Light light, vec3 L, vec3 N, vec3 V)
{
    L = normalize(L); N = normalize(N); V = normalize(V);
    vec3 ambient = light.colour * light.ambientIntensity;
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = light.colour * light.diffuseIntensity * NdotL;
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), material.shininess);
    vec3 specular = light.colour * material.specularIntensity * spec;
    return ambient + diffuse + specular;
}

float pcfShadow(sampler2D smap, vec3 proj, float bias)
{
    vec2 texel = 1.0 / textureSize(smap, 0);
    float s = 0.0;
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y) {
        float closest = texture(smap, proj.xy + vec2(x,y) * texel).r;
        s += (proj.z - bias > closest) ? 1.0 : 0.0;
    }
    return s / 9.0;
}

float CalcDirectionalShadowFactor(DirectionalLight light)
{
    vec3 proj = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
    proj = proj * 0.5 + 0.5;
    
    if (proj.x < 0.0 || proj.x > 1.0 ||
    proj.y < 0.0 || proj.y > 1.0 ||
    proj.z > 1.0)
    return 0.0;
    
    float ndotl = max(dot(normalize(Normal), normalize(-light.direction)), 0.0);
    float bias = max(0.0015 * (1.0 - ndotl), 0.0005);

    return pcfShadow(directionalShadowMap, proj, bias);
}

vec4 CalcDirectional(float shadow)
{
    vec3 L = normalize(-directionalLight.direction);
    vec3 V = normalize(eyePosition - FragPos);
    vec3 N = normalize(Normal);

    vec3 ambient = directionalLight.base.colour * directionalLight.base.ambientIntensity;
    vec3 ds = ApplyLight(directionalLight.base, L, N, V) - ambient; // diffuse+spec

    vec3 lit = ambient + (1.0 - shadow) * ds;
    return vec4(lit, 1.0);
}

vec3 CalcPointLight(PointLight pLight)
{
    vec3 V = normalize(eyePosition - FragPos);
    vec3 N = normalize(Normal);
    vec3 L = pLight.position - FragPos;
    float d = length(L);
    L /= max(d, 1e-6);
    vec3 c = ApplyLight(pLight.base, L, N, V);
    float atten = pLight.constant + pLight.linear * d + pLight.exponent * d * d;
    return c / max(atten, 1e-4);
}

vec4 CalcPointLights()
{
    vec3 total = vec3(0.0);
    for (int i = 0; i < pointLightCount; ++i)
    total += CalcPointLight(pointLights[i]);
    return vec4(total, 1.0);
}

vec3 CalcSpotLight(SpotLight sLight)
{
    vec3 L = normalize(FragPos - sLight.base.position);
    vec3 D = normalize(sLight.direction);
    float sl = dot(D, L);
    if (sl <= sLight.edge) return vec3(0.0);

    float denom = max(1.0 - sLight.edge, 1e-6);
    float spotFactor = clamp((sl - sLight.edge) / denom, 0.0, 1.0);

    return CalcPointLight(sLight.base) * spotFactor;
}

vec4 CalcSpotLights()
{
    vec3 total = vec3(0.0);
    for (int i = 0; i < spotLightCount; ++i)
    total += CalcSpotLight(spotLights[i]);
    return vec4(total, 1.0);
}

void main()
{
    vec3 albedo = texture(theTexture, TexCoord).rgb;
    
    float shadow = CalcDirectionalShadowFactor(directionalLight);
    vec4 dirCol   = CalcDirectional(shadow);
    vec4 pointsCol = CalcPointLights();
    vec4 spotsCol  = CalcSpotLights();

    colour = vec4(albedo * (dirCol.rgb + pointsCol.rgb + spotsCol.rgb), 1.0);
}
