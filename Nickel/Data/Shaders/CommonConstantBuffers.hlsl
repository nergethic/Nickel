cbuffer PerApplication : register(b0) {
	matrix projectionMatrix;
	float3 clientData;
}

cbuffer PerFrame : register(b1) {
	matrix viewMatrix;
	float3 eyePos;
	float3 lightPos;
}

cbuffer PerObject : register(b2) {
	matrix modelMatrix;
	matrix viewProjectionMatrix;
	matrix modelViewProjectionMatrix;
}