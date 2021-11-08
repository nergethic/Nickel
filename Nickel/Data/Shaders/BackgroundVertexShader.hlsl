cbuffer PerApplication : register( b0 )
{
	matrix projectionMatrix;
	float3 clientData;
}

cbuffer PerFrame : register( b1 )
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