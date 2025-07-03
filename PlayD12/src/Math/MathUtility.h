#pragma once

#include "Matrix.h" 

namespace MMath {

	float constexpr PI = 3.14159265358979323846f; // Pi constant
     

    //cross product 
	inline FLOAT3 Vector3Cross(FLOAT3 a, FLOAT3 b) {
		return {
			a.y() * b.z() - a.z() * b.y(),
			a.z() * b.x() - a.x() * b.z(),
			a.x() * b.y() - a.y() * b.x()
		};
	}



	inline FLOAT4X4 MatrixTranslation
	(
		float x, float y, float z
	)
	{
		FLOAT4X4 translation = MatrixIdentity<float, 4>();
		translation[3] = { x, y, z, 1.0f };
		translation = Transpose(translation); // Transpose for column-major order
		return translation;
	}

	inline FLOAT4X4 MatrixScaling
	(
		float x, float y, float z
	)
	{
		FLOAT4X4 scalingMatrix = MatrixIdentity<float, 4>();
		scalingMatrix[0] = { x, 0.0f, 0.0f, 0.0f };
		scalingMatrix[1] = { 0.0f, y, 0.0f, 0.0f };
		scalingMatrix[2] = { 0.0f, 0.0f, z, 0.0f };
		scalingMatrix[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
		return scalingMatrix;
	}




    inline FLOAT4X4 LookToLH
    (
        FLOAT3 EyePosition,
        FLOAT3 EyeDirection,
        FLOAT3 UpDirection
    )  
    {

        FLOAT3 forward = Normalize(EyeDirection);

        FLOAT3 right_N = Vector3Cross(UpDirection, forward);
        right_N = Normalize(right_N);

        FLOAT3 up_N = Vector3Cross(forward, right_N);

		FLOAT4X4 matrix = MatrixIdentity<float,4>();
         
		//init by column for convenience:
		matrix[0] = { right_N.x(), right_N.y(), right_N.z(), -Dot(right_N, EyePosition) };
		matrix[1] = { up_N.x(), up_N.y(), up_N.z(), -Dot(up_N, EyePosition) };
		matrix[2] = { forward.x(), forward.y(), forward.z(), -Dot(forward, EyePosition) };
		matrix[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
         
		matrix = Transpose(matrix); 

        return matrix;
    }


	inline FLOAT4X4 LookAtLH
	(
		FLOAT3 EyePosition,
		FLOAT3 TargetPosition,
		FLOAT3 UpDirection
	)
	{
		// Calculate the forward vector (looking direction)
		FLOAT3 forward = Normalize(TargetPosition - EyePosition);

		return  LookToLH(EyePosition, forward, UpDirection);
	}


	inline FLOAT4X4 PerspectiveFovLH
	(
		float fovAngleY,
		float aspectRatio,
		float nearZ,
		float farZ
	)
	{

		FLOAT4X4 perspectiveMatrix = MatrixIdentity<float, 4>();
		float yScale = 1.0f / tanf(fovAngleY * 0.5f);
		float xScale = yScale / aspectRatio;
		perspectiveMatrix[0] = { xScale, 0.0f, 0.0f, 0.0f };
		perspectiveMatrix[1] = { 0.0f, yScale, 0.0f, 0.0f };
		perspectiveMatrix[2] = {
			0.0f,
			0.0f,
			farZ / (farZ - nearZ),             
			-(nearZ * farZ) / (farZ - nearZ)    
		};
		perspectiveMatrix[3] = {
			0.0f,
			0.0f,
			1.0f,                                
			0.0f                             
		};

		 

		return perspectiveMatrix;
		 
	}

	inline float ToRadians(float degrees) {
		return degrees * (PI / 180.0f);
	}



}