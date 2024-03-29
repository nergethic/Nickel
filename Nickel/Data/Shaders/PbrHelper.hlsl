static const float PI = 3.14159265359;

inline float3 ToneMapHDR(const float3 color) {
    // Reinhard operator
    return color / (color + float3(1.0, 1.0, 1.0));
}

inline float3 CorrectGamma(const float3 color) {
    const float powN = 1.0 / 2.2;
    return pow(color, float3(powN, powN, powN));
}

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

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
    const float oneMinusRoughness = 1.0 - roughness;
    return F0 + (max(float3(oneMinusRoughness, oneMinusRoughness, oneMinusRoughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}