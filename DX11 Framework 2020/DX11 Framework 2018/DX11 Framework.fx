//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
    //update var
    float gTime;
    
    //view matrixs
	matrix World;
	matrix View;
	matrix Projection;

    //diffuse
    float4 DiffuseMtrl;
    float4 DiffuseLight;
    float3 LightVecW; //light pos

    //ambient
    float4 AmbientMtrl;
    float4 AmbientLight;

    //ambient
    float4 SpecularMtrl;
    float4 SpecularLight;
    float SpecularPower;

    //camera
    float3 EyePosW;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Eye : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 Normal : NORMAL )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    //Pos.xy += 0.5f * sin(Pos.x) * sin(3.0f * gTime);
    //Pos.z *= 0.6f + 0.4f * sin(2.0f * gTime);

    //model to world space
    output.Pos = mul( Pos, World );

    //set camera position and normalize
    output.Eye = normalize(EyePosW - output.Pos.xyz);

    //world space to view space
    output.Pos = mul( output.Pos, View );

    //view space to projection space
    output.Pos = mul( output.Pos, Projection );

    //calculate normals
    float3 calculatedNormal = mul(float4(Normal, 0.0f), World).xyz;
    output.Normal = normalize(calculatedNormal);

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    //compute reflection vector
    float r = reflect(-LightVecW, input.Normal);

    //calculate ambient light colour
    float3 ambient = AmbientMtrl * AmbientLight;

    //calculate specular light colour
    float specularAmount = pow(max(dot(r, input.Eye), 0), SpecularPower); //how much light hits the camera eye
    float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

    //diffuse calculate diffuse light colour
    float diffuseAmount = max(dot(LightVecW, input.Normal), 0.0f);
    float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;

    //calculate final pixel colour
    float4 finalColor;
    finalColor.rgb = clamp(diffuse, 0, 1) + ambient + clamp(specular, 0, 1); //clamps make sure values stay between 0 and 1
    finalColor.a = DiffuseMtrl.a;

    return finalColor;
    //return input.Normal; //check normals (change func to float3)
}
