cbuffer ConstantBuffer : register(b0) {
	matrix World;
	matrix View;
	matrix Projection;
}

struct VS_INPUT {
	float4 Pos : POSITION;
	float4 Color : COLOR;
};

struct PS_INPUT {
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input) {
	PS_INPUT output;
	float4x4 wvp = mul(World, mul(View, Projection));
	output.Pos = mul(input.Pos, wvp);

	output.Color = input.Color;
	return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET {
	return input.Color;
}
