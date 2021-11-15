static const float PI = 3.14159265359;

inline float3 ToneMapHDR(const float3 color) {
    // Reinhard operator
    return color / (color + float3(1.0, 1.0, 1.0));
}

inline float3 CorrectGamma(const float3 color) {
    const float powN = 1.0 / 2.2;
    return pow(color, float3(powN, powN, powN));
}