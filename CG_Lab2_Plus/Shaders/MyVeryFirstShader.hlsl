cbuffer VSConstants : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};

struct VS_IN {
	float4 pos : POSITION0;
	float4 col : COLOR0;
};

struct PS_IN {
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

PS_IN VSMain(VS_IN input) {
	PS_IN output;

	float4 worldPos = mul(world, input.pos);
	float4 viewPos  = mul(view, worldPos);
	output.pos = mul(projection, viewPos);

	output.col = input.col;
	return output;
}

float4 PSMain(PS_IN input) : SV_Target {
	#ifdef TEST
		if (input.pos.x > 400)
			return TCOLOR;
	#endif
	return input.col;
}
