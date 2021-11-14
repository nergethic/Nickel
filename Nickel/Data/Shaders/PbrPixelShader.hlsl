Texture2D albedo : register(t0);
SamplerState sampleType : register(s0);

const float PI = 3.14159265359;

cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
    float3 clientData;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
    float3 eyePos;
    float3 lightPos;
}

cbuffer ShaderData : register(b2)
{
    float3 lightPositions[4];
    float3 lightColors[4];
    float albedoFactor;
    float metallic;
    float roughness;
    float ao;
}

struct PixelShaderInput
{
    float3 worldPos : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float4 PbrPixelShader(PixelShaderInput IN) : SV_TARGET
{
    float3 N = normalize(IN.normal);
    float3 V = normalize(eyePos - IN.worldPos);

    //float3 albedoTexel = albedo.Sample(sampleType, IN.uv).rgb;
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), float3(albedoFactor, albedoFactor, albedoFactor), metallic);

    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 4; ++i) {
        // calculate per-light radiance
        float3 L = normalize(lightPositions[i] - IN.worldPos);
        float3 H = normalize(V + L);
        float distance = length(lightPositions[i] - IN.worldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = lightColors[i] * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedoFactor / PI + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.03, 0.03, 0.03) * albedoFactor * ao;
    float3 color = ambient + Lo;

    color = color / (color + float3(1.0, 1.0, 1.0));
    const float colorPow = 1.0 / 2.2;
    color = pow(color, float3(colorPow, colorPow, colorPow));

    return float4(color.rgb, 1.0);
}