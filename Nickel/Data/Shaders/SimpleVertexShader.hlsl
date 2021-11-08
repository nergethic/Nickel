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
	matrix modelViewMatrix;
	matrix modelViewProjectionMatrix;
}

struct VertexData
{
    float3 position : POSITION;
	float3 normal: NORMAL;
    float3 color: COLOR;
};

struct VertexShaderOutput
{
	float3 worldPos : WORLD_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float3 view : VIEW;
	float3 light : LIGHT;
	float4 position : SV_POSITION;
};

VertexShaderOutput SimpleVertexShader(VertexData IN )
{
    VertexShaderOutput OUT;

    OUT.position = mul(float4(IN.position, 1.0f), modelViewProjectionMatrix);
	OUT.worldPos = mul(float4(IN.position, 1.0f), modelMatrix).xyz;

	OUT.normal = mul(IN.normal, (float3x3)modelMatrix); // world space normal
	OUT.normal = normalize(IN.normal);
	//OUT.normal.z = -OUT.normal.z;

	OUT.color = float4( IN.color, 1.0f );

	//OUT.view = normalize(eyePos - OUT.worldPos);
	//OUT.light = normalize(lightPos - OUT.worldPos); //float3(10.0f, 0.0f, 0.0f);
	OUT.view = eyePos;
	OUT.light = lightPos;

    return OUT;
}