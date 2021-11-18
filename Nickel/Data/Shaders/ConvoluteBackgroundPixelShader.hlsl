TextureCube tex : register(t0);
SamplerState ss : register(s0);

#include "PbrHelper.hlsl"

float4 ConvoluteBackgroundPixelShader(float3 worldPos : WORLD_POSITION) : SV_TARGET
{
    float3 N = normalize(worldPos);
	float3 irradiance = float3(0.0, 0.0, 0.0);

    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += tex.Sample(ss, sampleVec).rgb * cos(theta)* sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
}