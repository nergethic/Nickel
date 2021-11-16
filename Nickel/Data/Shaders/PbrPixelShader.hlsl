Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D metalRoughnessTex : register(t2);
Texture2D aoTex : register(t3);
Texture2D emissionTex : register(t4);

TextureCube irradianceMap : register(t5);
TextureCube radianceMap : register(t6);

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
    float3 normalWS : NORMAL_WS;
    float2 uv : TEXCOORD0;
};

float3 GetNormalFromMap(float3 normal, float3 worldPos, float2 uv) {
    float3 tangentNormal = normalTex.Sample(sampleType, uv).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);

    float3 Q1 = ddx(worldPos);
    float3 Q2 = ddy(worldPos);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);

    //float3 Normal = normalize(normal);
    //float3 Tangent = normalize(Q1 * st2.y - Q2 * st1.y);
    //float3 Binormal = -normalize(cross(Normal, Tangent));

    float3 Normal = normalize(normal);
    float3 Tangent = -normalize(Q1 * st2.y - Q2 * st1.y);
    float3 Binormal = normalize(cross(Normal, Tangent));

    float3x3 TBN = float3x3(Tangent, Binormal, Normal);

    return normalize(mul(tangentNormal, TBN));
}

float4 PbrPixelShader(PixelShaderInput IN) : SV_TARGET
{
    const float3 albedoTexel = pow(albedoTex.Sample(sampleType, IN.uv).rgb, 2.2);
    const float2 metalRoughnessTexel = metalRoughnessTex.Sample(sampleType, IN.uv).rg;
    const float metallic = metalRoughnessTexel.r;
    const float roughness = metalRoughnessTexel.g;
    const float ao = aoTex.Sample(sampleType, IN.uv).r;
    const float3 emissionTexel = pow(emissionTex.Sample(sampleType, IN.uv).rgb, 2.2);

    float3 N = GetNormalFromMap(normalize(IN.normalWS), IN.worldPos, IN.uv);
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

    // float3 ambient =  // texture(irradianceMap, N).rgb;
    float3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0);
    float3 kD = 1.0 - kS;
    float3 irradiance = irradianceMap.Sample(sampleType, N).rgb;
    float3 diffuse = irradiance * albedoTexel;
    float3 ambient = (kD * diffuse) * ao;
    //float3 ambient = float3(0.13, 0.13, 0.13) * albedoTexel.rgb * ao;
    float3 color = ambient + Lo + emissionTexel;

    color = ToneMapHDR(color);
    color = CorrectGamma(color);

    return float4(color.rgb, 1.0);
}