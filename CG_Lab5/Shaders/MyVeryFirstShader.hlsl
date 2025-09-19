cbuffer VSConstants : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
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

	float3 cameraPos; // добавляем сюда положение камеры
	float padding;    // выравнивание
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
	float4 worldPos4 = mul(pos, world);
	output.WorldPos = worldPos4.xyz;
	output.Pos = mul(mul(worldPos4, view), proj);
	output.Normal = normalize(mul(float4(normal,0), world).xyz);
	output.Color = color;
	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	float3 N = normalize(input.Normal);
	float3 V = normalize(cameraPos - input.WorldPos); // используем cameraPos из буфера

	float3 finalColor = ambientColor * input.Color.rgb;

	for(int i=0; i<numLights; i++)
	{
		float3 L = normalize(lights[i].position - input.WorldPos);
		float diff = max(dot(N,L), 0.0);
		float3 diffuse = diff * input.Color.rgb * lights[i].color * lights[i].intensity;

		float3 R = reflect(-L,N);
		float spec = pow(max(dot(R,V),0.0),16);
		float3 specular = spec * lights[i].color * lights[i].intensity;

		finalColor += diffuse + specular;
	}
	return float4(finalColor,input.Color.a);
}
