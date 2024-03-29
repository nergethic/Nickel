cbuffer PerApplication : register( b0 )
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register( b1 )
{
    matrix viewMatrix;
	float3 eyePos;
	float3 lightPos;
}

cbuffer PerObject : register( b2 )
{
    matrix worldMatrix; // model matrix
}

struct AppData
{
    float3 position : POSITION;
    float3 normal: NORMAL;
	float2 uv: UV;
};

struct VertexShaderOutput
{
	float2 uv : UV;
	float4 position : SV_POSITION;
};

VertexShaderOutput TexVertexShader( AppData IN )
{
    VertexShaderOutput OUT;

	/*
	matrix mvp = mul( projectionMatrix, mul( viewMatrix, worldMatrix ) );
    OUT.position = mul( mvp, float4( IN.position, 1.0f ) );
	OUT.worldPos = mul(float4( IN.position, 1.0f ), worldMatrix);

	OUT.normal = mul(IN.normal, (float3x3)worldMatrix); // world space normal
	OUT.normal = normalize(IN.normal);
	//OUT.normal.z = -OUT.normal.z;

	OUT.uv = IN.uv;

	//OUT.view = normalize(eyePos - OUT.worldPos);
	//OUT.light = normalize(lightPos - OUT.worldPos); //float3(10.0f, 0.0f, 0.0f);
	//OUT.view = eyePos;
	OUT.light = lightPos;
	*/

	matrix mvp = mul( projectionMatrix, mul( viewMatrix, worldMatrix ) );
	OUT.position = mul( mvp, float4( IN.position, 1.0f ) );

	matrix modelViewMatrix = mul( viewMatrix, worldMatrix );
	float3 eye = mul(float4( IN.position, 1.0f ), modelViewMatrix);
	eye = normalize(eye);
	//float3 normal = normalize(IN.normal);
	float3 normal = mul(IN.normal, (float3x3)worldMatrix); // world space normal
	normal = normalize(IN.normal);

	float3 r = reflect(eye, normal);
	float m = 2.0 * sqrt(
		pow( r.x, 2.0 ) +
		pow( r.y, 2.0 ) +
		pow( r.z + 1.0, 2.0 )
	);
	OUT.uv = r.xy / m + 0.5;

    return OUT;
}