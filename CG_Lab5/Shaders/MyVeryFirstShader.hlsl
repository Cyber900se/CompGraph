cbuffer WorldOnly : register(b0)
{
    matrix WorldMatrix;
};

cbuffer VSConstants : register(b1)
{
    matrix world;
    matrix view;
    matrix proj;
    matrix worldInvTranspose; 
};

cbuffer PSConstants : register(b2)
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

};

cbuffer LightMatrices : register(b3)
{
    matrix LightView;
    matrix LightProjection;
};

struct VS_OUTPUT_DEPTH
{
    float4 Pos      : SV_POSITION;
};

struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float4 Color    : COLOR0;
    float3 Normal   : NORMAL;
    float3 WorldPos : POSITION1;
    float4 ShadowPos : TEXCOORD0;
};

Texture2D ShadowMap : register(t0); 
SamplerComparisonState ShadowSampler : register(s0); // Comparison Sampler для PCF

VS_OUTPUT_DEPTH LightVSMain(float4 pos : POSITION)
{
    VS_OUTPUT_DEPTH output;

    float4 worldPos = mul(pos, WorldMatrix);

    float4 lightViewPos = mul(worldPos, LightView);
    output.Pos = mul(lightViewPos, LightProjection);
    
    return output;
}

VS_OUTPUT VSMain(float4 pos : POSITION, float4 color : COLOR, float3 normal : NORMAL)
{
    VS_OUTPUT output;

    float4 worldPos4 = mul(pos, world);
    output.WorldPos = worldPos4.xyz;
    output.Pos = mul(mul(worldPos4, view), proj);

    float3 normalW = mul(normal, (float3x3)worldInvTranspose);
    output.Normal = normalize(normalW); 

    output.Color = color;

    float4 lightViewPos = mul(worldPos4, LightView);

    output.ShadowPos = mul(lightViewPos, LightProjection);
    
    return output;
}

float CalculateShadowFactor(float4 shadowPos)
{
    // Переводим из clip space [-1,1] → UV [0,1]
    float2 projTexCoord = 0.5f * (shadowPos.xy / shadowPos.w) + 0.5f;
    float compareDepth  = shadowPos.z / shadowPos.w;

    // Вне shadow map → нет тени
    if (projTexCoord.x < 0.0f || projTexCoord.x > 1.0f ||
        projTexCoord.y < 0.0f || projTexCoord.y > 1.0f)
        return 1.0f;

    // Размер texel'а для PCF
    float2 texelSize = 1.0f / float2(1024.0f, 1024.0f);

    // Небольшой bias, чтобы убрать acne
    float bias = 0.005f;

    // PCF 2x2
    float result = 0.0f;
    result += ShadowMap.SampleCmpLevelZero(ShadowSampler, projTexCoord + texelSize * float2(-0.5f, -0.5f), compareDepth + bias);
    result += ShadowMap.SampleCmpLevelZero(ShadowSampler, projTexCoord + texelSize * float2( 0.5f, -0.5f), compareDepth + bias);
    result += ShadowMap.SampleCmpLevelZero(ShadowSampler, projTexCoord + texelSize * float2(-0.5f,  0.5f), compareDepth + bias);
    result += ShadowMap.SampleCmpLevelZero(ShadowSampler, projTexCoord + texelSize * float2( 0.5f,  0.5f), compareDepth + bias);

    return saturate(result * 0.25f); // clamp 0..1
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    float3 N = normalize(input.Normal);
    float3 V = normalize(cameraPos - input.WorldPos);

    float shadowFactor = CalculateShadowFactor(input.ShadowPos);

    float3 finalColor = ambientColor * input.Color.rgb;

    for (int i = 0; i < numLights; i++)
    {
       float3 L = normalize(lights[i].position - input.WorldPos);

       float diff = max(dot(N, L), 0.0);
       float3 diffuse = diff * input.Color.rgb * lights[i].color * lights[i].intensity;

       float3 R = reflect(-L, N);
       float spec = (diff > 0.0) ? pow(max(dot(R, V), 0.0), 16) : 0.0;
       float3 specular = spec * lights[i].color * lights[i].intensity;

       finalColor += (diffuse + specular) * shadowFactor; 
    }

    return float4(finalColor, input.Color.a);
}