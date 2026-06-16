cbuffer TriangleTransformConstants : register(b0) {
    row_major float4x4 u_world;
    row_major float4x4 u_view_projection;
};

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput vs_main(VSInput input) {
    PSInput output;
    output.position = mul(mul(float4(input.position, 1.0), u_world), u_view_projection);
    output.color = input.color;
    return output;
}

float4 ps_main(PSInput input) : SV_TARGET {
    return input.color;
}
