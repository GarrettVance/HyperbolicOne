//      
//      
//      Constant buffer stores three column-major matrices: 
//      
//      


cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};



struct VertexShaderInput
{
    float3      pos     :       POSITION;
    float2      tex     :       TEXCOORD;
    float3      normal  :       NORMAL;
};






struct PixelShaderInput
{
	float4      s_pos       : SV_POSITION;
	float3      s_color     : COLOR0;
    float2      s_texco     : TEXCOORD0;
};





PixelShaderInput vs_main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos4 = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.

	float4 worldpos = mul(pos4, model);
	float4 viewpos = mul(worldpos, view);
	float4 projpos = mul(viewpos, projection);

	output.s_pos = projpos;

    output.s_color = float3(0.f, 0.f, 1.f);

    //   
    //  Poincare Disk texture coordinates: 
    //   

    output.s_texco = float2(input.pos.x, input.pos.y);  //  bypasses the Model Transform;

	return output;
}
