#pragma once
#include <array>
#include <span>
#include <cassert>
#include <concepts>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <execution>
#include <ranges>


#include "Vector.h"

namespace MMath {
	 

	template <FLOP_t Scalar_t, std::size_t Rows, std::size_t Cols>
	class Matrix {
	public:
		using VectorType = Vector<Scalar_t, Rows>;

		static constexpr std::size_t RowCount = Rows;
		static constexpr std::size_t ColCount = Cols;
		static constexpr std::size_t ElementCount = Rows * Cols;

		// Default zero-init constructor
		Matrix() noexcept {
			VectorType zeroVector{};
			for (std::size_t i = 0; i < Cols; ++i) {
				m_data[i] = zeroVector;
			}
		}

		// From initializer list of column vectors
		Matrix(std::initializer_list<Vector<Scalar_t, Rows>> columns) noexcept {
			assert(columns.size() == Cols);
			std::copy(columns.begin(), columns.end(), m_data.begin());
		}

		//accepts spans:
		Matrix(std::span<Vector<Scalar_t, Rows>, Cols> columns) noexcept {
			assert(columns.size() == Cols);
			std::copy(columns.begin(), columns.end(), m_data.begin());
		}



		// Access column vector
		const Vector<Scalar_t, Rows>& operator[](std::size_t col) const {
			assert(col < Cols);
			return m_data[col];
		}

		Vector<Scalar_t, Rows>& operator[](std::size_t col) {
			assert(col < Cols);
			return m_data[col];
		}



	private:
		// Column-major: m_data[Col] = Vector<Rows>
		std::array<Vector<Scalar_t, Rows>, Cols> m_data;
	};



	//----------------------------------------------------------
	//transpose:
	template <FLOP_t T, std::size_t Rows, std::size_t Cols>
	inline Matrix<T, Cols, Rows> Transpose(const Matrix<T, Rows, Cols>& mat) {
		Matrix<T, Cols, Rows> result;
		for (std::size_t r = 0; r < Rows; ++r) {
			for (std::size_t c = 0; c < Cols; ++c) {
				result[c][r] = mat[r][c];
			}
		}
		return result;
	}



	//----------------------------------------------------------
	// Identity matrix
	template <FLOP_t T, std::size_t N>
	Matrix<T, N, N> MatrixIdentity() {
		Matrix<T, N, N> result;
		for (std::size_t i = 0; i < N; ++i) {
			result[i][i] = T{ 1 };
		}
		return result;
	}

	//----------------------------------------------------------
	// Matrix-Vector multiplication;  columen major;  reuse dot product

	template <FLOP_t T, std::size_t Rows, std::size_t Cols>
	Vector<T, Cols> MatrixVectorMultiply(const Matrix<T, Rows, Cols>& mat, const Vector<T, Cols>& vec) {
		static_assert(Cols == Cols, "Matrix-Vector multiplication requires compatible dimensions.");
		Vector<T, Rows> result;
		for (std::size_t c = 0; c < Cols; ++c) {
			result[c] = std::inner_product(mat[c].data().begin(), mat[c].data().end(), vec.data().begin(), T{});
		}
		return result;
	}


	template <FLOP_t T, std::size_t Rows, std::size_t Cols>
	Vector<T, Rows> operator*(const Matrix<T, Rows, Cols>& mat, const Vector<T, Cols>& vec) { 
		static_assert(Cols == Rows, "Matrix-Vector multiplication requires compatible dimensions.");
		return MatrixVectorMultiply(mat, vec);
	}

	//----------------------------------------------------------
	// Matrix-Matrix multiplication; reuse matrix-vector multiplication
	template <FLOP_t T, std::size_t A, std::size_t B, std::size_t C>
	Matrix<T, A, C> MatrixMultiply(const Matrix<T, A, B>& lhs, const Matrix<T, B, C>& rhs) {
		static_assert(B == A, "Matrix multiplication requires compatible dimensions.");
		Matrix<T, A, C> result;
		for (std::size_t c = 0; c < C; ++c) {
			result[c] = MatrixVectorMultiply(rhs, lhs[c]);  //row-major storage;
		}
		result = Transpose(result); 
		return result;
	}


	template <FLOP_t T, std::size_t A, std::size_t B, std::size_t C>
	Matrix<T, A, C> operator*(const Matrix<T, A, B>& lhs, const Matrix<T, B, C>& rhs) {
		static_assert(B == A, "Matrix multiplication requires compatible dimensions.");  
		return MatrixMultiply(lhs, rhs);
	}




	 


	//----------------------------------------------------------
	using FLOAT2X2 = Matrix<float, 2, 2>;
	using DOUBLE2X2 = Matrix<double, 2, 2>; 
	using FLOAT3X3 = Matrix<float, 3, 3>;
	using FLOAT4X4 = Matrix<float, 4, 4>;
	using DOUBLE3X3 = Matrix<double, 3, 3>;
	using DOUBLE4X4 = Matrix<double, 4, 4>;
}
