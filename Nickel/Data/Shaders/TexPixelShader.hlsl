Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct PixelShaderInput
{
	float2 uv : UV;
};

float4 TexPixelShader(PixelShaderInput IN) : SV_TARGET
{
	float2 uv = IN.uv;
	//uv.x = 1.0 - uv.x;
	//uv.y = 1.0 + uv.y;

	float3 textureColor = shaderTexture.Sample(sampleType, uv).rgb;

	return float4(textureColor, 1.0);
	//return float4(saturate(IN.uv.x), saturate(IN.uv.y), 0.0, 1.0f);
}