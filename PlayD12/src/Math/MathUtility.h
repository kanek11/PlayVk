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

	inline Float4x4 InverseLookToLH(
		Float3 EyePosition,
		Float3 EyeDirection,
		Float3 UpDirection
	)
	{
		Float3 forward = Normalize(EyeDirection); 
		Float3 right_N = Vector3Cross(UpDirection, forward);
		right_N = Normalize(right_N); 
		Float3 up_N = Vector3Cross(forward, right_N);
		 
		Float4x4 inv = MatrixIdentity<float, 4>();
		//3x3 is transpose,  translation is minus:
		inv[0] = { right_N.x(), up_N.x(), forward.x(), EyePosition.x() };
		inv[1] = { right_N.y(), up_N.y(), forward.y(), EyePosition.y() };
		inv[2] = { right_N.z(), up_N.z(), forward.z(), EyePosition.z() };
		inv[3] = { 0.0f, 0.0f, 0.0f, 1.0f };        


		inv = Transpose(inv); 

		return inv;
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


	inline Float4x4 InverseLookAtLH(
		Float3 EyePosition,
		Float3 TargetPosition,
		Float3 UpDirection
	)
	{
		// Calculate the forward vector (looking direction)
		Float3 forward = Normalize(TargetPosition - EyePosition);
		return InverseLookToLH(EyePosition, forward, UpDirection);
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
	 
	inline Float4x4 InversePerspectiveFovLH(
		float fovAngleY,
		float aspectRatio,
		float nearZ,
		float farZ
	)
	{
		Float4x4 inv = MatrixIdentity<float, 4>();
		float yScale = 1.0f / tanf(fovAngleY * 0.5f);
		float xScale = yScale / aspectRatio;
		float A = farZ / (farZ - nearZ);
		float B = -(nearZ * farZ) / (farZ - nearZ);
		 
		inv[0] = { 1.0f / xScale, 0.0f,     0.0f,      0.0f };
		inv[1] = { 0.0f, 1.0f / yScale,     0.0f,      0.0f };
		inv[2] = { 0.0f, 0.0f,              0.0f,      1.0f / B};
		inv[3] = { 0.0f, 0.0f,              1.0f,      -A / B };
		 

		return inv;
	}



	inline Float4x4 OrthographicOffCenterLH(
		float left, float right,
		float bottom, float top,
		float nearZ, float farZ)
	{
		Float4x4 ortho = MatrixIdentity<float, 4>();

		float rl = right - left;
		float tb = top - bottom;
		float fn = farZ - nearZ;

		ortho[0] = { 2.0f / rl, 0.0f,      0.0f,        0.0f };
		ortho[1] = { 0.0f,      2.0f / tb, 0.0f,        0.0f };
		ortho[2] = { 0.0f,      0.0f,      1.0f / fn,   0.0f };
		ortho[3] = { -(right + left) / rl, -(top + bottom) / tb, -nearZ / fn, 1.0f };

		return ortho;
	}



	inline Float4x4 OrthographicLH
	(
		float width,
		float height,
		float nearZ,
		float farZ
	)
	{
		float halfW = width * 0.5f;
		float halfH = height * 0.5f;
		return OrthographicOffCenterLH(-halfW, halfW, -halfH, halfH, nearZ, farZ);
	}


	inline float ToRadians(float degrees) {
		return degrees * (PI / 180.0f);
	}


	inline float Det3x3(const Float3x3& m)
	{
		return m[0].x() * (m[1].y() * m[2].z() - m[1].z() * m[2].y()) -
			m[0].y() * (m[1].x() * m[2].z() - m[1].z() * m[2].x()) +
			m[0].z() * (m[1].x() * m[2].y() - m[1].y() * m[2].x());
	}

	//matrix3x3 inverse
	inline Float3x3 Inverse3x3(const Float3x3& m)
	{
		Float3x3 inv;
		float det = Det3x3(m);
		if (det < 1e-07) {
			std::cerr << "Inverse: Matrix is (almost) singular." << std::endl;
			return Float3x3{};  
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
	 
	inline Float4x4 Inverse4x4(const Float4x4& m)
	{
		Float4x4 inv;
		float* invOut = reinterpret_cast<float*>(&inv);
		const float* mat = reinterpret_cast<const float*>(&m);

		invOut[0] = mat[5] * mat[10] * mat[15] -
			mat[5] * mat[11] * mat[14] -
			mat[9] * mat[6] * mat[15] +
			mat[9] * mat[7] * mat[14] +
			mat[13] * mat[6] * mat[11] -
			mat[13] * mat[7] * mat[10];

		invOut[4] = -mat[4] * mat[10] * mat[15] +
			mat[4] * mat[11] * mat[14] +
			mat[8] * mat[6] * mat[15] -
			mat[8] * mat[7] * mat[14] -
			mat[12] * mat[6] * mat[11] +
			mat[12] * mat[7] * mat[10];

		invOut[8] = mat[4] * mat[9] * mat[15] -
			mat[4] * mat[11] * mat[13] -
			mat[8] * mat[5] * mat[15] +
			mat[8] * mat[7] * mat[13] +
			mat[12] * mat[5] * mat[11] -
			mat[12] * mat[7] * mat[9];

		invOut[12] = -mat[4] * mat[9] * mat[14] +
			mat[4] * mat[10] * mat[13] +
			mat[8] * mat[5] * mat[14] -
			mat[8] * mat[6] * mat[13] -
			mat[12] * mat[5] * mat[10] +
			mat[12] * mat[6] * mat[9];

		invOut[1] = -mat[1] * mat[10] * mat[15] +
			mat[1] * mat[11] * mat[14] +
			mat[9] * mat[2] * mat[15] -
			mat[9] * mat[3] * mat[14] -
			mat[13] * mat[2] * mat[11] +
			mat[13] * mat[3] * mat[10];

		invOut[5] = mat[0] * mat[10] * mat[15] -
			mat[0] * mat[11] * mat[14] -
			mat[8] * mat[2] * mat[15] +
			mat[8] * mat[3] * mat[14] +
			mat[12] * mat[2] * mat[11] -
			mat[12] * mat[3] * mat[10];

		invOut[9] = -mat[0] * mat[9] * mat[15] +
			mat[0] * mat[11] * mat[13] +
			mat[8] * mat[1] * mat[15] -
			mat[8] * mat[3] * mat[13] -
			mat[12] * mat[1] * mat[11] +
			mat[12] * mat[3] * mat[9];

		invOut[13] = mat[0] * mat[9] * mat[14] -
			mat[0] * mat[10] * mat[13] -
			mat[8] * mat[1] * mat[14] +
			mat[8] * mat[2] * mat[13] +
			mat[12] * mat[1] * mat[10] -
			mat[12] * mat[2] * mat[9];

		invOut[2] = mat[1] * mat[6] * mat[15] -
			mat[1] * mat[7] * mat[14] -
			mat[5] * mat[2] * mat[15] +
			mat[5] * mat[3] * mat[14] +
			mat[13] * mat[2] * mat[7] -
			mat[13] * mat[3] * mat[6];

		invOut[6] = -mat[0] * mat[6] * mat[15] +
			mat[0] * mat[7] * mat[14] +
			mat[4] * mat[2] * mat[15] -
			mat[4] * mat[3] * mat[14] -
			mat[12] * mat[2] * mat[7] +
			mat[12] * mat[3] * mat[6];

		invOut[10] = mat[0] * mat[5] * mat[15] -
			mat[0] * mat[7] * mat[13] -
			mat[4] * mat[1] * mat[15] +
			mat[4] * mat[3] * mat[13] +
			mat[12] * mat[1] * mat[7] -
			mat[12] * mat[3] * mat[5];

		invOut[14] = -mat[0] * mat[5] * mat[14] +
			mat[0] * mat[6] * mat[13] +
			mat[4] * mat[1] * mat[14] -
			mat[4] * mat[2] * mat[13] -
			mat[12] * mat[1] * mat[6] +
			mat[12] * mat[2] * mat[5];

		invOut[3] = -mat[1] * mat[6] * mat[11] +
			mat[1] * mat[7] * mat[10] +
			mat[5] * mat[2] * mat[11] -
			mat[5] * mat[3] * mat[10] -
			mat[9] * mat[2] * mat[7] +
			mat[9] * mat[3] * mat[6];

		invOut[7] = mat[0] * mat[6] * mat[11] -
			mat[0] * mat[7] * mat[10] -
			mat[4] * mat[2] * mat[11] +
			mat[4] * mat[3] * mat[10] +
			mat[8] * mat[2] * mat[7] -
			mat[8] * mat[3] * mat[6];

		invOut[11] = -mat[0] * mat[5] * mat[11] +
			mat[0] * mat[7] * mat[9] +
			mat[4] * mat[1] * mat[11] -
			mat[4] * mat[3] * mat[9] -
			mat[8] * mat[1] * mat[7] +
			mat[8] * mat[3] * mat[5];

		invOut[15] = mat[0] * mat[5] * mat[10] -
			mat[0] * mat[6] * mat[9] -
			mat[4] * mat[1] * mat[10] +
			mat[4] * mat[2] * mat[9] +
			mat[8] * mat[1] * mat[6] -
			mat[8] * mat[2] * mat[5];

		float det = mat[0] * invOut[0] + mat[1] * invOut[4] + mat[2] * invOut[8] + mat[3] * invOut[12];

		if (det == 0.0f) {
			std::cerr << "Matrix is singular, cannot invert!" << std::endl;
			return Float4x4{}; 
		}

		det = 1.0f / det;
		for (int i = 0; i < 16; ++i)
			invOut[i] *= det;

		return inv;
	}



	inline Float3x3 ToFloat3x3(const Float4x4& mat) {
		Float3x3 result;
		result[0] = { mat[0][0], mat[0][1], mat[0][2] };
		result[1] = { mat[1][0], mat[1][1], mat[1][2] };
		result[2] = { mat[2][0], mat[2][1], mat[2][2] };
		return result; 
	}

	inline Float4x4 ToFloat4x4(const Float3x3& mat) {
		Float4x4 result = MatrixIdentity<float, 4>();
		result[0] = { mat[0][0], mat[0][1], mat[0][2], 0.0f };
		result[1] = { mat[1][0], mat[1][1], mat[1][2], 0.0f };
		result[2] = { mat[2][0], mat[2][1], mat[2][2], 0.0f };
		result[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
		return result;
	}




	inline std::string XMToString(const DirectX::XMMATRIX& mat) {
		std::ostringstream oss;
		for (int i = 0; i < 4; ++i) {
			oss << std::format("{: .3f}, {: .3f}, {: .3f}, {: .3f}\n",
				mat.r[i].m128_f32[0], mat.r[i].m128_f32[1],
				mat.r[i].m128_f32[2], mat.r[i].m128_f32[3]);
		}
		return oss.str();
	}

	inline std::string XMToString(const DirectX::XMVECTOR& vec) {
		return std::format("[{:.3f}, {:.3f}, {:.3f}, {:.3f}]",
			vec.m128_f32[0], vec.m128_f32[1], vec.m128_f32[2], vec.m128_f32[3]);
	}



	inline std::vector<Float3> GenerateGrid3D(Float3 dim , Float3 spacing)
	{
		std::vector<Float3> data; 

		for (int x = 0; x < dim.x(); ++x)
			for (int y = 0; y < dim.y(); ++y)
				for (int z = 0; z < dim.z(); ++z)
		{ 
			Float3 _data = Float3{ x * spacing.x()  , y * spacing.y() , z * spacing.z() };
			data.push_back(_data);
		}

		return data;
	}


	template <FLOP_t Scalar_t, std::size_t Rows, std::size_t Cols>
	inline Scalar_t FNorm(const Matrix<Scalar_t, Rows, Cols>& mat)
	{
		Scalar_t sum = 0;
		for (std::size_t i = 0; i < Rows; ++i) {
			for (std::size_t j = 0; j < Cols; ++j) {
				sum += mat[i][j] * mat[i][j];
			}
		}
		return std::sqrt(sum);
	}
	 
	template <FLOP_t Scalar_t, std::size_t Rows, std::size_t Cols>
	inline bool NearZero(const Matrix<Scalar_t, Rows, Cols>& mat, Scalar_t epsilon = 1e-6f)
	{
		return FNorm(mat) < epsilon;
	}

	inline Float3x3 QuaternionToRotationMatrix(const DirectX::XMVECTOR& q)
	{
		using namespace DirectX;
		// Convert quaternion to rotation matrix
		XMMATRIX R_ = XMMatrixRotationQuaternion(q);
		Float3x3 R;
		R[0] = { R_.r[0].m128_f32[0], R_.r[0].m128_f32[1], R_.r[0].m128_f32[2] };
		R[1] = { R_.r[1].m128_f32[0], R_.r[1].m128_f32[1], R_.r[1].m128_f32[2] };
		R[2] = { R_.r[2].m128_f32[0], R_.r[2].m128_f32[1], R_.r[2].m128_f32[2] };
		return R;
	} 


  
}


namespace Random {

	inline float Uniform01() {
		static thread_local std::mt19937 gen{ std::random_device{}() };
		static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		return dist(gen);
	}

	inline float UniformRange(float min, float max) {
		static thread_local std::mt19937 gen{ std::random_device{}() };
		static thread_local std::uniform_real_distribution<float> dist(min, max);
		return dist(gen);
	}
}