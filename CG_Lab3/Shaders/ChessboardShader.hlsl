struct VS_INPUT {
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT VS_Main(VS_INPUT input) {
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1.0);
    output.uv = input.uv;
    return output;
}

float4 PS_Main(VS_OUTPUT input) : SV_TARGET {
    int cellX = (int)(input.uv.x * 8);
    int cellY = (int)(input.uv.y * 8);
    bool isWhite = (cellX + cellY) % 2 == 0;
    return isWhite ? float4(1,1,1,1) : float4(0,0,0,1);
}