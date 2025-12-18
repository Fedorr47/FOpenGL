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
struct OmniShadowMap { samplerCube shadowMap; float farPlane; }; 

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
uniform OmniShadowMap OmniShadowMaps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

vec3 sampleOffsetDirections[20] = vec3[](
vec3( 1, 1, 1), vec3( 1,-1, 1), vec3(-1,-1, 1), vec3(-1, 1, 1),
vec3( 1, 1,-1), vec3( 1,-1,-1), vec3(-1,-1,-1), vec3(-1, 1,-1),
vec3( 1, 1, 0), vec3( 1,-1, 0), vec3(-1,-1, 0), vec3(-1, 1, 0),
vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0,-1), vec3(-1, 0,-1),
vec3( 0, 1, 1), vec3( 0,-1, 1), vec3( 0,-1,-1), vec3( 0, 1,-1)
);

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

float CalcOmniShadowFactor(vec3 lightPos, int shadowIndex)
{
    vec3 fragToLight   = FragPos - lightPos;
    float currentDepth = length(fragToLight);

    float farPlane     = OmniShadowMaps[shadowIndex].farPlane;

    float viewDistance = length(eyePosition - FragPos);
    float diskRadius   = (1.0 + (viewDistance / farPlane)) / 25.0;

    float bias   = 0.15;
    float shadow = 0.0;

    const int samples = 20;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(
        OmniShadowMaps[shadowIndex].shadowMap,
        fragToLight + sampleOffsetDirections[i] * diskRadius
        ).r;

        closestDepth *= farPlane;
        if (currentDepth - bias > closestDepth)
        shadow += 1.0;
    }

    shadow /= float(samples);
    return shadow;
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

vec3 CalcPointLight(PointLight pLight, float shadow)
{
    vec3 V = normalize(eyePosition - FragPos);
    vec3 N = normalize(Normal);

    vec3 L = pLight.position - FragPos;
    float d = length(L);
    L /= max(d, 1e-6);

    vec3 c = ApplyLight(pLight.base, L, N, V);

    float atten = pLight.constant + pLight.linear * d + pLight.exponent * d * d;
    c /= max(atten, 1e-4);
    
    vec3 ambient = pLight.base.colour * pLight.base.ambientIntensity;
    vec3 ds = c - ambient;
    return ambient + (1.0 - shadow) * ds;
}

vec4 CalcPointLights()
{
    vec3 total = vec3(0.0);
    for (int i = 0; i < pointLightCount; ++i)
    {
        float shadow = CalcOmniShadowFactor(pointLights[i].position, i);
        
        total += CalcPointLight(pointLights[i], shadow);
    }
    return vec4(total, 1.0);
}

vec3 CalcSpotLight(SpotLight sLight, int shadowIndex)
{
    vec3 toFrag = normalize(FragPos - sLight.base.position);

    vec3 D = normalize(sLight.direction);
    float sl = dot(D, toFrag);

    if (sl <= sLight.edge) return vec3(0.0);

    float denom = max(1.0 - sLight.edge, 1e-6);
    float spotFactor = clamp((sl - sLight.edge) / denom, 0.0, 1.0);

    float shadow = CalcOmniShadowFactor(sLight.base.position, shadowIndex);

    return CalcPointLight(sLight.base, shadow) * spotFactor;
}

vec4 CalcSpotLights()
{
    vec3 total = vec3(0.0);
    for (int i = 0; i < spotLightCount; ++i)
    {
        int shadowIndex = pointLightCount + i;
        total += CalcSpotLight(spotLights[i], shadowIndex);
    }
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
