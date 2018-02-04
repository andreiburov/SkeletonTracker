#include "SimpleSkeleton3D.hlsli"

cbuffer vertexConstantBuffer : register(b0)
{
	matrix worldView;
	matrix projection;
};


PixelShaderInput createSkeletonVertex(GeometryShaderInput input)
{
	PixelShaderInput output = (PixelShaderInput)0;
	float4 pos = mul(input.pos, worldView);
	output.pos = mul(pos, projection);
	output.col = float4(0, 0, 0, 1);
	return output;
}

PixelShaderInput createAxisEnd(GeometryShaderInput input)
{
	PixelShaderInput output = (PixelShaderInput)0;
	float4 pos = input.pos;
	if (!(input.axisAngle.x == 0 && input.axisAngle.y == 0 && input.axisAngle.z == 0))
	{
		pos += float4(input.axisAngle.xyz * 0.1, 1.f);
	}
	pos = mul(pos, worldView);
	output.pos = mul(pos, projection);
	output.col = float4(1, 0, 0, 1);
	return output;
}

//[maxvertexcount(4)]
[maxvertexcount(2)]
void main(
	line GeometryShaderInput input[2], 
	inout LineStream< PixelShaderInput > output
)
{
	output.Append(createSkeletonVertex(input[0]));
	output.Append(createSkeletonVertex(input[1]));

	//PixelShaderInput axisStart = createSkeletonVertex(input[1]);
	//axisStart.col = float4(1, 0, 0, 1);
	//output.Append(axisStart);
	//output.Append(createAxisEnd(input[1]));
}