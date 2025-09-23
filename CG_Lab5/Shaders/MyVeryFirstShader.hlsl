cbuffer VSConstants : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
	matrix worldInvTranspose; // новая матрица для нормалей
};

cbuffer PSConstants : register(b1)
{
	float3 ambientColor;
	int numLights;

	struct Light
	{
		float3 position;
		float intensity;
		float3 color;
		float padding;
	};
	Light lights[100];

	float3 cameraPos;
	float padding;

	float4 padding2;
};

struct VS_OUTPUT
{
	float4 Pos      : SV_POSITION;
	float4 Color    : COLOR0;
	float3 Normal   : NORMAL;
	float3 WorldPos : POSITION1;
};

VS_OUTPUT VSMain(float4 pos : POSITION, float4 color : COLOR, float3 normal : NORMAL)
{
	VS_OUTPUT output;

	// --- позиция ---
	float4 worldPos4 = mul(pos, world);
	output.WorldPos = worldPos4.xyz;
	output.Pos = mul(mul(worldPos4, view), proj);

	// --- нормали ---
	float3 normalW = mul(normal, (float3x3)worldInvTranspose);
	output.Normal = normalize(normalW);

	output.Color = color;
	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	float3 N = normalize(input.Normal);
	float3 V = normalize(cameraPos - input.WorldPos);

	float3 finalColor = ambientColor * input.Color.rgb;

	for (int i = 0; i < numLights; i++)
	{
		float3 L = normalize(lights[i].position - input.WorldPos);

		// диффуз
		float diff = max(dot(N, L), 0.0);
		float3 diffuse = diff * input.Color.rgb * lights[i].color * lights[i].intensity;

		// спекуляр (только если diff > 0)
		float3 R = reflect(-L, N);
		float spec = (diff > 0.0) ? pow(max(dot(R, V), 0.0), 16) : 0.0;
		float3 specular = spec * lights[i].color * lights[i].intensity;

		finalColor += diffuse + specular;
	}

	return float4(finalColor, input.Color.a);
}
