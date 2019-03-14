Texture2D shaderTexture;
SamplerState sampleType;

struct PixelShaderInput
{
	float2 uv : UV;
};

float4 TexPixelShader( PixelShaderInput IN ) : SV_TARGET
{
	float3 textureColor = shaderTexture.Sample(sampleType, IN.uv).rgb;

	return float4(textureColor, 1.0);
	//return float4(saturate(IN.uv.x), saturate(IN.uv.y), 0.0, 1.0f);
}