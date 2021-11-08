float4 ColorPixelShader(float4 color : COLOR) : SV_TARGET {
	return float4(color.rgb, 1.0f);
}