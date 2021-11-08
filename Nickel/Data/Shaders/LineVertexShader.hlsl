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

cbuffer ShaderData : register(b3)
{
    float thickness;
    int miter;
}

struct VertexData
{
    float3 position : POSITION;
    float3 previous : POSITION2;
    float3 next : POSITION3;
    float3 direction : DIRECTION;
};

struct VertexShaderOutput
{
	float4 color : COLOR;
	float4 position : SV_POSITION;
};

VertexShaderOutput LineVertexShader(VertexData IN)
{
    VertexShaderOutput OUT;

    //float3 previousWorld = mul(float4(IN.previous, 1.0f), modelMatrix);
    //float3 currentWorld = mul(float4(IN.position, 1.0f), modelMatrix);
    //float3 nextWorld = mul(float4(IN.next, 1.0f), modelMatrix);

    //float4 previousProjected = mul(float4(previousWorld, 1.0), viewProjectionMatrix);
    //float4 currentProjected = mul(float4(currentWorld, 1.0), viewProjectionMatrix);
    //float4 nextProjected = mul(float4(nextWorld, 1.0), viewProjectionMatrix);

    float4 previousProjected = mul(float4(IN.previous, 1.0f), modelViewProjectionMatrix);
    float4 currentProjected = mul(float4(IN.position, 1.0f), modelViewProjectionMatrix);
    float4 nextProjected = mul(float4(IN.next, 1.0f), modelViewProjectionMatrix);

    const float aspectRatio = clientData[2];
    const float2 aspectVec = float2(aspectRatio, 1.0f);
    //get 2D screen space with W divide and aspect correction
    float2 currentScreen = (currentProjected.xy / currentProjected.w) * aspectVec;
    float2 previousScreen = (previousProjected.xy / previousProjected.w) * aspectVec;
    float2 nextScreen = (nextProjected.xy / nextProjected.w) * aspectVec;

    // OUT.color = float4((currentScreen.x+1.0)/2.0, (currentScreen.y+1.0)/2.0f, 0.0, 1.0);

    float len = thickness;

    // NOTE: starting point uses (next - current)
    float2 dir = float2(0.0f, 0.0f);
    if (currentScreen.x == previousScreen.x && currentScreen.y == previousScreen.y) {
        dir = normalize(nextScreen - currentScreen);
    } else if (currentScreen.x == nextScreen.x && currentScreen.y == nextScreen.y) { //ending point uses (current - previous)
        dir = normalize(currentScreen - previousScreen);
    } else {
        // NOTE: somewhere in middle, needs a join
        // NOTE: get dir from (C - B) and (B - A)
        float2 dirA = normalize(currentScreen - previousScreen);
        if (miter == 1) {
            float2 dirB = normalize(nextScreen - currentScreen);

            float2 tangent = normalize(dirA + dirB);
            float2 perp = float2(-dirA.y, dirA.x);
            float2 miter = float2(-tangent.y, tangent.x);
            dir = tangent;
            len = thickness / dot(miter, perp);
        } else {
            dir = dirA;
        }
    }

    float2 normal = normalize(float2(-dir.y, dir.x));
    normal *= len * 0.5f;
    normal.x /= aspectRatio;

    const float orientation = IN.direction[0];
    const float2 offset = normal * orientation;
    OUT.position = currentProjected + float4(offset.xy, 0.0, 0.0);
    OUT.color = float4(0.9, 0.9, 0.9, 1.0);

    return OUT;
}