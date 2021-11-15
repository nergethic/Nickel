#include "CommonConstantBuffers.hlsl"

struct VertexShaderOutput
{
	float3 worldPos : WORLD_POSITION;
	float4 position : SV_POSITION;
};

VertexShaderOutput BackgroundVertexShader(float3 pos : WORLD_POSITION)
{
    VertexShaderOutput OUT;
	OUT.position = mul(float4(pos, 0.0f), viewProjectionMatrix); // NOTE: 0.0f cancels translation - only getting rotation part

	// NOTE: depth after 'w' divide needs to be 1.0 (for z-buffer comparison)
	OUT.position.z = OUT.position.w;
	OUT.worldPos = pos;

    return OUT;
}