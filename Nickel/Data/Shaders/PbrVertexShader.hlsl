#include "CommonConstantBuffers.hlsl"

struct VertexData
{
	float3 position : POSITION;
	float3 normal: NORMAL;
	float3 uv: TEXCOORD0;
};

struct VertexShaderOutput
{
	float3 worldPos : WORLD_POSITION;
	float3 normalWS : NORMAL_WS;
	float2 uv : TEXCOORD0;
	float4 position : SV_POSITION;
};

VertexShaderOutput PbrVertexShader(VertexData IN)
{
	VertexShaderOutput OUT;

	OUT.position = mul(float4(IN.position, 1.0f), modelViewProjectionMatrix);
	OUT.worldPos = mul(float4(IN.position, 1.0f), modelMatrix).xyz;

	OUT.normalWS = normalize(mul(IN.normal, (float3x3)modelMatrix)); // world space normal
	
	OUT.uv = IN.uv;

	return OUT;
}