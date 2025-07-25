#pragma once

//numeric limits:
#include <limits>

#include "Matrix.h" 

namespace MMath {

	float constexpr PI = 3.14159265358979323846f; // Pi constant

	//max possible float value:
	float constexpr FLOAT_MAX = std::numeric_limits<float>::max();


	template <typename T, size_t N>
	std::string ToString(const Vector<T, N>& vec) {
		std::ostringstream oss;
		oss << "[";
		for (size_t i = 0; i < N; ++i) {
			oss << vec[i];
			if (i != N - 1) oss << ", ";
		}
		oss << "]";
		return oss.str();
	}

	template <typename T, size_t Rows, size_t Cols>
	std::string ToString(const Matrix<T, Rows, Cols>& mat) {
		std::ostringstream oss;
		for (int i = 0; i < 4; ++i) {
			oss << std::format("{: .3f}, {: .3f}, {: .3f}, {: .3f}\n",
				mat[i][0], mat[i][1], mat[i][2], mat[i][3]);
		}
		return oss.str(); 
	}




	//cross product 
	inline Float3 Vector3Cross(Float3 a, Float3 b) {
		return {
			a.y() * b.z() - a.z() * b.y(),
			a.z() * b.x() - a.x() * b.z(),
			a.x() * b.y() - a.y() * b.x()
		};
	}



	inline Float4x4 MatrixTranslation
	(
		float x, float y, float z
	)
	{
		Float4x4 translation = MatrixIdentity<float, 4>();
		translation[3] = { x, y, z, 1.0f };
		
		//std::cout << "translation matrix: " << ToString(translation) << std::endl;


		//translation = Transpose(translation);  
		return translation;
	}

	inline Float4x4 MatrixScaling
	(
		float x, float y, float z
	)
	{
		Float4x4 scalingMatrix = MatrixIdentity<float, 4>();
		scalingMatrix[0] = { x, 0.0f, 0.0f, 0.0f };
		scalingMatrix[1] = { 0.0f, y, 0.0f, 0.0f };
		scalingMatrix[2] = { 0.0f, 0.0f, z, 0.0f };
		scalingMatrix[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
		return scalingMatrix;
	}




	inline Float4x4 LookToLH
	(
		Float3 EyePosition,
		Float3 EyeDirection,
		Float3 UpDirection
	)
	{

		Float3 forward = Normalize(EyeDirection);

		Float3 right_N = Vector3Cross(UpDirection, forward);
		right_N = Normalize(right_N);

		Float3 up_N = Vector3Cross(forward, right_N);

		Float4x4 matrix = MatrixIdentity<float, 4>();

		//init by column for convenience:
		matrix[0] = { right_N.x(), right_N.y(), right_N.z(), -Dot(right_N, EyePosition) };
		matrix[1] = { up_N.x(), up_N.y(), up_N.z(), -Dot(up_N, EyePosition) };
		matrix[2] = { forward.x(), forward.y(), forward.z(), -Dot(forward, EyePosition) };
		matrix[3] = { 0.0f, 0.0f, 0.0f, 1.0f }; 

		matrix = Transpose(matrix); 
		return matrix;
	}


	inline Float4x4 LookAtLH
	(
		Float3 EyePosition,
		Float3 TargetPosition,
		Float3 UpDirection
	)
	{
		// Calculate the forward vector (looking direction)
		Float3 forward = Normalize(TargetPosition - EyePosition);

		return  LookToLH(EyePosition, forward, UpDirection);
	}


	inline Float4x4 PerspectiveFovLH
	(
		float fovAngleY,
		float aspectRatio,
		float nearZ,
		float farZ
	)
	{

		Float4x4 perspectiveMatrix = MatrixIdentity<float, 4>();
		float yScale = 1.0f / tanf(fovAngleY * 0.5f);
		float xScale = yScale / aspectRatio;
		perspectiveMatrix[0] = { xScale, 0.0f, 0.0f, 0.0f };
		perspectiveMatrix[1] = { 0.0f, yScale, 0.0f, 0.0f };
		perspectiveMatrix[2] = {0.0f,0.0f,farZ / (farZ - nearZ), 1.0 };
		perspectiveMatrix[3] = {0.0f,0.0f, -(nearZ * farZ) / (farZ - nearZ),0.0f };

		return perspectiveMatrix;

	}

	inline Float4x4 OrthographicLH
	(
		float width,
		float height,
		float nearZ,
		float farZ
	)
	{
		Float4x4 orthoMatrix = MatrixIdentity<float, 4>();
		orthoMatrix[0] = { 2.0f / width, 0.0f, 0.0f, 0.0f };
		orthoMatrix[1] = { 0.0f, 2.0f / height, 0.0f, 0.0f };
		orthoMatrix[2] = { 0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f };
		orthoMatrix[3] = { 0.0f, 0.0f, -nearZ / (farZ - nearZ), 1.0f };
		return orthoMatrix;
	}


	inline float ToRadians(float degrees) {
		return degrees * (PI / 180.0f);
	}


	//matrix3x3 inverse
	inline Float3x3 Inverse(const Float3x3& m)
	{
		Float3x3 inv;
		float det = m[0].x() * (m[1].y() * m[2].z() - m[1].z() * m[2].y()) -
			m[0].y() * (m[1].x() * m[2].z() - m[1].z() * m[2].x()) +
			m[0].z() * (m[1].x() * m[2].y() - m[1].y() * m[2].x());
		if (det == 0.0f) {
			std::cerr << "Matrix is singular." << std::endl;
			return Float3x3{}; // return zero matrix if singular
		}
		float invDet = 1.0f / det;
		inv[0] = {
			invDet * (m[1].y() * m[2].z() - m[1].z() * m[2].y()),
			invDet * (m[0].z() * m[2].y() - m[0].y() * m[2].z()),
			invDet * (m[0].y() * m[1].z() - m[0].z() * m[1].y())
		};
		inv[1] = {
			invDet * (m[1].z() * m[2].x() - m[1].x() * m[2].z()),
			invDet * (m[0].x() * m[2].z() - m[0].z() * m[2].x()),
			invDet * (m[0].z() * m[1].x() - m[0].x() * m[1].z())
		};
		inv[2] = {
			invDet * (m[1].x() * m[2].y() - m[1].y() * m[2].x()),
			invDet * (m[0].y() * m[2].x() - m[0].x() * m[2].y()),
			invDet * (m[0].x() * m[1].y() - m[0].y() * m[1].x())
		};

		return inv;
	}





	inline std::string XMMatrixToString(const DirectX::XMMATRIX& mat) {
		std::ostringstream oss;
		for (int i = 0; i < 4; ++i) {
			oss << std::format("{: .3f}, {: .3f}, {: .3f}, {: .3f}\n",
				mat.r[i].m128_f32[0], mat.r[i].m128_f32[1],
				mat.r[i].m128_f32[2], mat.r[i].m128_f32[3]);
		}
		return oss.str();
	}


}