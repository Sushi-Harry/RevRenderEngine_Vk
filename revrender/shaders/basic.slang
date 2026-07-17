
struct PushConstants {
    float4x4 viewProj;
    float4x4 model;
};

[[vk::push_constant]] 
ConstantBuffer<PushConstants> push;

struct VertexInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

// --- VERTEX SHADER ---
[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    
    // Slang uses mul() for matrix multiplication, just like HLSL
    float4 worldPos = mul(push.model, float4(input.position, 1.0));
    output.position = mul(push.viewProj, worldPos);
    output.texCoord = input.texCoord;
    
    return output;
}

[[vk::binding(0, 0)]] 
Sampler2D crateTexture;

// --- FRAGMENT SHADER ---
[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    // Sample the texture using the UV coordinates
    return crateTexture.Sample(input.texCoord);
}