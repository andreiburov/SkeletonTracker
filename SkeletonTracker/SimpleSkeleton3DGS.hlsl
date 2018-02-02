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
		element.pos = input[i].pos;
		output.Append(element);
	}
}