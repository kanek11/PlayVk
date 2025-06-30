#pragma once

#include <concepts> 
#include <initializer_list>
#include <cassert>
#include <span> 
#include <type_traits>
#include <algorithm> // std::copy, std::fill, std::transform
#include <iterator>  
#include <execution>
#include <numeric>  // std::transform


namespace MMath {

	//----------------------------------------------------------
	//concept that is floating point type:
	template <typename T>
	concept FLOP_t = std::is_floating_point_v<T>;
	 
	//----------------------------------------------------------
	template <FLOP_t Scalar_t, uint32_t Length> 
	class Vector
	{
	public:
		static constexpr uint32_t LengthValue = Length; 
		static constexpr uint32_t SizeInBytes = Length * sizeof(Scalar_t); 

		//constructors: 
		//explicit Zero init 
		Vector() noexcept { std::fill(std::begin(m_data), std::end(m_data), Scalar_t{}); }
		Vector(std::initializer_list<Scalar_t> values) noexcept
		{
			assert(values.size() == Length && "Initializer list size must match vector length.");
			std::copy(values.begin(), values.end(), m_data);
		}
		//accepts spans
		Vector(std::span<Scalar_t, Length> values) noexcept
		{
			assert(values.size() == Length && "Span size must match vector length.");
			std::copy(values.begin(), values.end(), m_data);
		} 
		//the other 4 are defaulted;
		//Vector(const Vector& other) noexcept = default; 
		//Vector(Vector&& other) noexcept = default;  
		//Vector& operator=(const Vector& other) noexcept = default; 
		//Vector& operator=(Vector&& other) noexcept = default;  
 


		//access
		Scalar_t operator[](uint32_t i) const { assert(i < Length); return m_data[i]; }
		Scalar_t& operator[](uint32_t i) { assert(i < Length); return m_data[i]; }

		std::span<const Scalar_t> data() const { return std::span{ m_data }; }
		std::span<Scalar_t> data() { return std::span{ m_data }; }

		Scalar_t x() const requires (Length > 0) { return m_data[0]; } 
		Scalar_t y() const requires (Length > 1) { return m_data[1]; } 
		Scalar_t z() const requires (Length > 2) { return m_data[2]; } 
		Scalar_t w() const requires (Length > 3) { return m_data[3]; } 

		auto xy() const requires (Length >= 2) {
			return Vector<Scalar_t, 2>{m_data[0], m_data[1]};
		}

		auto xyz() const requires (Length >= 3) {
			return Vector<Scalar_t, 3>{m_data[0], m_data[1], m_data[2]};
		}   

		//swizzle is also supported like this;

	private:
		Scalar_t m_data[Length]; 
	};
	 

	//----------------------------------------------------------
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> VectorAdd(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{
		Vector<Scalar_t, Length> result;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result[i] = lhs[i] + rhs[i];
		}
		return result;
	}
	 
	inline Vector<float, 3> operator+(const Vector<float, 3>& lhs, const Vector<float, 3>& rhs)
	{
		return VectorAdd(lhs, rhs);
	}

	//----------------------------------------------------------
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> VectorSubtract(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{
		Vector<Scalar_t, Length> result;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result[i] = lhs[i] - rhs[i];
		}
		return result;
	}
	 
	inline Vector<float, 3> operator-(const Vector<float, 3>& lhs, const Vector<float, 3>& rhs)
	{
		return VectorSubtract(lhs, rhs);
	}

	//----------------------------------------------------------
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> VectorMultiply(const Vector<Scalar_t, Length>& vector, Scalar_t multipler)
	{ 
		Vector<Scalar_t, Length> result;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result[i] = vector[i] * multipler;
		}
		return result;
	}

	//facade operator overload:
	inline Vector<float, 3> operator*(const Vector<float, 3>& vector, float multipler)
	{
		return VectorMultiply(vector, multipler);
	}

	//----------------------------------------------------------
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> VectorDivide(const Vector<Scalar_t, Length>& vector, Scalar_t divisor)
	{
		assert(divisor != 0 && "Division by zero is not allowed.");
		Vector<Scalar_t, Length> result = VectorMultiply(vector, static_cast<Scalar_t>(1/ divisor) );
		return result;
	}
	 
	inline Vector<float, 3> operator/(const Vector<float, 3>& vector, float divisor)
	{
		return VectorDivide(vector, divisor);
	}


	//----------------------------------------------------------
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Scalar_t Dot(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{ 
		Scalar_t result = 0;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result += lhs[i] * rhs[i];
		}
		return result;
	}


	//----------------------------------------------------------
	//length squared:
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Scalar_t LengthSq(const Vector<Scalar_t, Length>& vector)
	{
		Scalar_t result = 0;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result += vector[i] * vector[i];
		}
		return result;
	}

	//----------------------------------------------------------
	//length: 
	template <FLOP_t Scalar_t, uint32_t Length_v>
	inline Scalar_t Length(const Vector<Scalar_t, Length_v>& vector)
	{
		Scalar_t lengthSq = LengthSq(vector);
		assert(lengthSq >= 0 && "Length squared cannot be negative.");
		return std::sqrt(lengthSq);
	}

	//----------------------------------------------------------
	//get normalized:
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> Normalize(const Vector<Scalar_t, Length>& vector)
	{ 
		Scalar_t length = std::sqrt(LengthSq(vector));
		assert(length > 0 && "Cannot normalize a zero-length vector.");
		return VectorDivide(vector, length);
	}

	//----------------------------------------------------------
	//common factory:
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> ZeroVector()
	{
		Vector<Scalar_t, Length> result;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result[i] = static_cast<Scalar_t>(0);
		}
		return result;
	}

	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> OneVector()
	{
		Vector<Scalar_t, Length> result;
		for (uint32_t i = 0; i < Length; ++i)
		{
			result[i] = static_cast<Scalar_t>(1);
		}
		return result;
	}

	//x axis:
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> XUnitVector()
	{
		Vector<Scalar_t, Length> result;
		if constexpr (Length > 0) {
			result[0] = static_cast<Scalar_t>(1);
		}
		for (uint32_t i = 1; i < Length; ++i)
		{
			result[i] = static_cast<Scalar_t>(0);
		}
		return result;
	}
	 
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> YUnitVector()
	{
		Vector<Scalar_t, Length> result;
		if constexpr (Length > 1) {
			result[1] = static_cast<Scalar_t>(1);
		}
		for (uint32_t i = 0; i < Length; ++i)
		{
			if (i != 1) {
				result[i] = static_cast<Scalar_t>(0);
			}
		}
		return result;
	}

	template <FLOP_t Scalar_t, uint32_t Length>
	inline Vector<Scalar_t, Length> ZUnitVector()
	{
		Vector<Scalar_t, Length> result;
		if constexpr (Length > 2) {
			result[2] = static_cast<Scalar_t>(1);
		}
		for (uint32_t i = 0; i < Length; ++i)
		{
			if (i != 2) {
				result[i] = static_cast<Scalar_t>(0);
			}
		}
		return result;
	}


	//----------------------------------------------------------
	using FLOAT2 = Vector<float, 2>;
	using FLOAT3 = Vector<float, 3>;
	using FLOAT4 = Vector<float, 4>; 

	using DOUBLE2 = Vector<double, 2>;
	using DOUBLE3 = Vector<double, 3>;
	using DOUBLE4 = Vector<double, 4>;
}