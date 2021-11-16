TextureCube tex : register(t0);
SamplerState ss : register(s0);

#include "PbrHelper.hlsl"

/*
float3 cubemapSeamlessFixDirection(float3 dir, const float scale) {
    // http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
    float M = max(max(abs(dir.x), abs(dir.y)), abs(dir.z));

    if (abs(dir.x) != M)
        dir.x *= scale;

    if (abs(dir.y) != M)
        dir.y *= scale;

    if (abs(dir.z) != M)
        dir.z *= scale;

    return dir;
}

float4 textureCubemap(const in samplerCube tex, const in float3 dir) {
    float4 rgba = textureCube(tex, dir);
    return LogLuvToLinear(rgba);
}

// Seamless cubemap for background
float4 textureCubeFixed(const samplerCube tex, const float3 direction) {
    // http://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
    float scale = 1.0 - 1.0 / uEnvironmentSize[0];
    vec3 dir = cubemapSeamlessFixDirection(direction, scale);
    return textureCubemap(tex, dir);
}
*/

float4 BackgroundPixelShader(float3 worldPos : WORLD_POSITION) : SV_TARGET{
    float3 color = tex.Sample(ss, worldPos);

    color = ToneMapHDR(color);
    // color = CorrectGamma(color);

    return float4(color, 1.0);
}