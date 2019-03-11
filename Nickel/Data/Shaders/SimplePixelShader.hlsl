struct PixelShaderInput
{
	float3 worldPos : WORLD_POSITION;
    float4 color : COLOR;
	float3 normal : NORMAL;
	float3 eyeVec : VIEW;
	float3 lightVec : LIGHT;
};

float4 SimplePixelShader( PixelShaderInput IN ) : SV_TARGET
{
	float3 n = float3(IN.normal.x, IN.normal.y, IN.normal.z);
	//float3 v = IN.eyeVec;
	//float3 l = IN.lightVec;

	float3 v = normalize(IN.eyeVec - IN.worldPos);
	float3 l = normalize(IN.lightVec - IN.worldPos);

	float3 cool = float3(0.0, 0.0, 0.1) + 0.1*IN.color.rgb;
	float3 warm = IN.color.rgb;
	float3 highlight = float3(1.0, 1.0, 1.0);

	float t = (dot(n,l)+1.0)/2.0;
	float3 r = 2.0*dot(n,l)*n-l;
	//float3 r = reflect(-l, n);
	float s = saturate(100.0*dot(r,v) - 97.0);

	float3 c = s*highlight + ((1.0-s)*(t*warm + (1.0-t)*cool));
	//float3 c = t*warm + (1.0-t)*cool;
	//float3 c = lerp(cool, warm, t);
	//float3 c = lerp(highlight, lerp(warm, cool, t), 1.0-s);
    return float4(c, 1.0f);

	//return float4(IN.view, 1.0f);
	//return IN.color;
	//return float4(1.0, 0.5, 0.0, 1.0f);
}