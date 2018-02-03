#include "SimpleSkeleton3D.hlsli"

cbuffer vertexConstantBuffer : register(b0)
{
	matrix worldView;
	matrix projection;
};

[maxvertexcount(2)]
void main(
	line GeometryShaderInput input[2], 
	inout LineStream< PixelShaderInput > output
)
{
	for (uint i = 0; i < 2; i++)
	{
		PixelShaderInput element = (PixelShaderInput)0;
		float4 pos = mul(input[i].pos, worldView);
		element.pos = mul(pos, projection);
		output.Append(element);
	}
}