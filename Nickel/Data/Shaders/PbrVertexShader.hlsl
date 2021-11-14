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

cbuffer PerObject : register(b2)
{
	matrix modelMatrix;
	matrix viewProjectionMatrix;
	matrix modelViewProjectionMatrix;
}

struct VertexData
{
	float3 position : POSITION;
	float3 normal: NORMAL;
	float3 uv: TEXCOORD0;
};

struct VertexShaderOutput
{
	float3 worldPos : WORLD_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float4 position : SV_POSITION;
};

VertexShaderOutput PbrVertexShader(VertexData IN)
{
	VertexShaderOutput OUT;

	OUT.position = mul(float4(IN.position, 1.0f), modelViewProjectionMatrix);
	OUT.worldPos = mul(float4(IN.position, 1.0f), modelMatrix).xyz;

	OUT.normal = mul(IN.normal, (float3x3)modelMatrix); // world space normal
	OUT.normal = normalize(IN.normal);
	//OUT.normal.z = -OUT.normal.z;
	OUT.uv = IN.uv;

	return OUT;
}