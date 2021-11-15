Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D metalRoughnessTex : register(t2);
Texture2D aoTex : register(t3);

SamplerState sampleType : register(s0);

#include "PbrHelper.hlsl"
#include "CommonConstantBuffers.hlsl"

cbuffer ShaderData : register(b3)
{
    float4 lightPositions[4];
    float4 lightColors[4];
    float4 albedoFactor;
    float metallicFactor;
    float roughnessFactor;
    float aoFactor;
}

struct PixelShaderInput
{
    float3 worldPos : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
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
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float4 PbrPixelShader(PixelShaderInput IN) : SV_TARGET
{
    const float3 albedoTexel = pow(albedoTex.Sample(sampleType, IN.uv), 2.2);
    const float2 metalRoughnessTexel = metalRoughnessTex.Sample(sampleType, IN.uv).rg;
    const float metallic = metalRoughnessTexel.r;
    const float roughness = metalRoughnessTexel.g;
    const float ao = aoTex.Sample(sampleType, IN.uv).r;

    float3 N = normalize(IN.normal);
    float3 V = normalize(eyePos - IN.worldPos);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedoTexel.rgb, metallic);

    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 4; ++i) {
        // calculate per-light radiance
        float3 L = normalize(lightPositions[i].xyz - IN.worldPos);
        float3 H = normalize(V + L);
        float distance = length(lightPositions[i].xyz - IN.worldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = lightColors[i].xyz * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

        float3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        float3 specular = numerator / denominator;

        // kS is equal to Fresnel
        float3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedoTexel.rgb / PI + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.13, 0.13, 0.13) * albedoTexel.rgb * ao;
    float3 color = ambient + Lo;

    color = ToneMapHDR(color);
    color = CorrectGamma(color);

    return float4(color.rgb, 1.0);
}