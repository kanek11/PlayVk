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
	template <FLOP_t Scalar_t, std::size_t Length>
	class Vector
	{
	public:
		static constexpr std::size_t LengthValue = Length;
		static constexpr std::size_t SizeInBytes = Length * sizeof(Scalar_t);

		//constructors: 
		//explicit Zero init 
		//Vector() noexcept { std::fill(std::begin(m_data), std::end(m_data), Scalar_t{}); } 

		constexpr Vector() noexcept {
			std::ranges::fill(m_data, Scalar_t{});
		}

		Vector(std::initializer_list<Scalar_t> values) noexcept
		{
			assert(values.size() == Length && "Initializer list size must match vector length.");
			std::copy(values.begin(), values.end(), m_data.begin());
		}
		//accepts spans
		Vector(std::span<Scalar_t, Length> values) noexcept
		{
			assert(values.size() == Length && "Span size must match vector length.");
			std::copy(values.begin(), values.end(), m_data.begin());
		}

		template <typename... Args>
			requires (sizeof...(Args) == Length && std::conjunction_v<std::is_convertible<Args, Scalar_t>...>)
		Vector(Args... args) noexcept
			: m_data{ static_cast<Scalar_t>(args)... } {
		}


		//the other 4 are defaulted;
		//Vector(const Vector& other) noexcept = default; 
		//Vector(Vector&& other) noexcept = default;  
		//Vector& operator=(const Vector& other) noexcept = default; 
		//Vector& operator=(Vector&& other) noexcept = default;  



		//access
		// 
		//fail on execution
		Scalar_t operator[](size_t i) const { assert(i < Length); return m_data[i]; }
		Scalar_t& operator[](size_t i) { assert(i < Length); return m_data[i]; }

		std::span<const Scalar_t> data() const { return std::span{ m_data }; }
		std::span<Scalar_t> data() { return std::span{ m_data }; }


		//fail on compilation
		template <std::size_t I>
		Scalar_t& get()& {
			static_assert(I < Length);
			return m_data[I];
		}

		template <std::size_t I>
		const Scalar_t& get() const& {
			static_assert(I < Length);
			return m_data[I];
		}

		template <std::size_t I>
		Scalar_t get() const&& {
			static_assert(I < Length);
			return m_data[I];
		}


		//support for 3D semantics:
		//readonly
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
		/*Scalar_t m_data[Length]; */
		std::array<Scalar_t, Length> m_data; // Using std::array for better safety and performance
	};



	//----------------------------------------------------------

	//equality :
	template <FLOP_t T, std::size_t N>
	inline bool VectorEqual(const Vector<T, N>& a, const Vector<T, N>& b) {
		if constexpr (N == 0) {
			return true; // Both are empty vectors
		}
		else {
			return std::equal(a.data().begin(), a.data().end(), b.data().begin());
		}
	}

	template <FLOP_t T, std::size_t N>
	inline bool operator==(const Vector<T, N>& a, const Vector<T, N>& b) {
		return VectorEqual(a, b);
	}


	template <FLOP_t Scalar_t, size_t Length>
	inline Vector<Scalar_t, Length> VectorAdd(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{
		Vector<Scalar_t, Length> result;
		//for (uint32_t i = 0; i < Length; ++i)
		//{
		//	result[i] = lhs[i] + rhs[i];
		//} 
		std::transform(lhs.data().begin(), lhs.data().end(), rhs.data().begin(), result.data().begin(),
			[](Scalar_t a, Scalar_t b) { return a + b; });

		return result;
	}

	template <FLOP_t T, std::size_t N>
	inline Vector<T, N> operator+(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
		return VectorAdd(lhs, rhs);
	}

	//+=:
	template <FLOP_t T, std::size_t N>
	inline Vector<T, N>& operator+=(Vector<T, N>& lhs, const Vector<T, N>& rhs) {
		for (size_t i = 0; i < N; ++i) {
			lhs[i] += rhs[i];
		}
		return lhs;
	}

	//----------------------------------------------------------
	template <FLOP_t Scalar_t, size_t Length>
	inline Vector<Scalar_t, Length> VectorSubtract(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{
		Vector<Scalar_t, Length> result;
		//for (uint32_t i = 0; i < Length; ++i)
		//{
		//	result[i] = lhs[i] - rhs[i];
		//}
		std::transform(lhs.data().begin(), lhs.data().end(), rhs.data().begin(), result.data().begin(),
			[](Scalar_t a, Scalar_t b) { return a - b; });
		return result;
	}

	template <FLOP_t T, std::size_t N>
	inline Vector<T, N> operator-(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
		return VectorSubtract(lhs, rhs);
	}


	//-=:
	template <FLOP_t T, std::size_t N>
	inline Vector<T, N>& operator-=(Vector<T, N>& lhs, const Vector<T, N>& rhs) {
		for (size_t i = 0; i < N; ++i) {
			lhs[i] -= rhs[i];
		}
		return lhs;
	}


	//----------------------------------------------------------
	template <FLOP_t Scalar_t, size_t Length>
	inline Vector<Scalar_t, Length> VectorMultiply(const Vector<Scalar_t, Length>& vector, Scalar_t multipler)
	{
		Vector<Scalar_t, Length> result;
		//for (uint32_t i = 0; i < Length; ++i)
		//{
		//	result[i] = vector[i] * multipler;
		//}
		std::transform(vector.data().begin(), vector.data().end(), result.data().begin(),
			[multipler](Scalar_t a) { return a * multipler; });
		return result;
	}

	//facade operator overload:
	template <FLOP_t T, std::size_t N>
	inline Vector<T, N> operator*(const Vector<T, N>& vector, T multipler)
	{
		return VectorMultiply(vector, multipler);
	}

	//swap order:
	template <FLOP_t T, std::size_t N>
	inline Vector<T, N> operator*(T multipler, const Vector<T, N>& vector)
	{
		return VectorMultiply(vector, multipler);
	}

	//*=:
	template <FLOP_t T, std::size_t N>
	inline Vector<T, N>& operator*=(Vector<T, N>& vector, T multipler)
	{
		for (size_t i = 0; i < N; ++i) {
			vector[i] *= multipler;
		}
		return vector;
	}

	//----------------------------------------------------------
	//negate
	template <FLOP_t Scalar_t, size_t Length>
	inline Vector<Scalar_t, Length> VectorNegate(const Vector<Scalar_t, Length>& vector)
	{
		Vector<Scalar_t, Length> result;
		std::transform(vector.data().begin(), vector.data().end(), result.data().begin(),
			[](Scalar_t a) { return -a; });
		return result;
	}

	//negate operator overload:
	template <FLOP_t T, std::size_t N>
	inline Vector<T, N> operator-(const Vector<T, N>& vector)
	{
		return VectorNegate(vector);
	}

	//----------------------------------------------------------
	template <FLOP_t Scalar_t, size_t Length>
	inline Vector<Scalar_t, Length> VectorDivide(const Vector<Scalar_t, Length>& vector, Scalar_t divisor)
	{
		assert(divisor != 0 && "Division by zero is not allowed.");
		Vector<Scalar_t, Length> result = VectorMultiply(vector, Scalar_t{ 1 } / divisor);
		return result;
	}

	template <FLOP_t T, std::size_t N>
	inline Vector<T, N> operator/(const Vector<T, N>& vector, T divisor)
	{
		assert(divisor != 0 && "Division by zero is not allowed.");
		return VectorDivide(vector, divisor);
	}

	// HadamardMultiply
	template <FLOP_t Scalar_t, size_t Length>
	inline Vector<Scalar_t, Length> HadamardMultiply(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{
		Vector<Scalar_t, Length> result;
		std::transform(lhs.data().begin(), lhs.data().end(), rhs.data().begin(), result.data().begin(),
			[](Scalar_t a, Scalar_t b) { return a * b; });
		return result;
	}


	//----------------------------------------------------------
	template <FLOP_t Scalar_t, size_t Length>
	inline Scalar_t Dot(const Vector<Scalar_t, Length>& lhs, const Vector<Scalar_t, Length>& rhs)
	{
		//Scalar_t result = 0;
		//for (uint32_t i = 0; i < Length; ++i)
		//{
		//	result += lhs[i] * rhs[i];
		//}
		Scalar_t result = std::inner_product(lhs.data().begin(), lhs.data().end(), rhs.data().begin(), Scalar_t{});

		return result;
	}


	//----------------------------------------------------------
	//length squared:
	template <FLOP_t Scalar_t, uint32_t Length>
	inline Scalar_t LengthSq(const Vector<Scalar_t, Length>& vector)
	{
		//Scalar_t result = 0;
		//for (uint32_t i = 0; i < Length; ++i)
		//{
		//	result += vector[i] * vector[i];
		//}
		Scalar_t result = std::inner_product(vector.data().begin(), vector.data().end(), vector.data().begin(), Scalar_t{});

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
	using Float2 = Vector<float, 2>;
	using Float3 = Vector<float, 3>;
	using Float4 = Vector<float, 4>;

	using DOUBLE2 = Vector<double, 2>;
	using DOUBLE3 = Vector<double, 3>;
	using DOUBLE4 = Vector<double, 4>;
}


// Structured binding customization point (std::get + tuple traits) 
namespace std {
	using namespace MMath;

	template <typename T, std::size_t N>
	struct tuple_size<Vector<T, N>> : std::integral_constant<std::size_t, N> {};

	template <std::size_t I, typename T, std::size_t N>
	struct tuple_element<I, Vector<T, N>> {
		static_assert(I < N);
		using type = T;
	};

	template <std::size_t I, typename T, std::size_t N>
	constexpr T& get(Vector<T, N>& v) noexcept {
		return v.template get<I>();
	}

	template <std::size_t I, typename T, std::size_t N>
	constexpr const T& get(const Vector<T, N>& v) noexcept {
		return v.template get<I>();
	}
}



namespace Color {
	using namespace MMath;

	static Float4 White = Float4(1.0f, 1.0f, 1.0f, 1.0f);
	static Float4 Black = Float4(0.0f, 0.0f, 0.0f, 1.0f);

	static Float4 Red = Float4(1.0f, 0.0f, 0.0f, 1.0f);
	static Float4 Green = Float4(0.0f, 1.0f, 0.0f, 1.0f);
	static Float4 Blue = Float4(0.0f, 0.0f, 1.0f, 1.0f);

	static Float4 Purple = Float4(1.0f, 0.0f, 1.0f, 1.0f);
	static Float4 Yellow = Float4(1.0f, 1.0f, 0.0f, 1.0f);
	static Float4 Cyan = Float4(0.0f, 1.0f, 1.0f, 1.0f);

	static Float4 Orange = Float4(1.0f, 0.5f, 0.0f, 1.0f);
	static Float4 Pink = Float4(1.0f, 0.75f, 0.8f, 1.0f);
	static Float4 Brown = Float4(0.6f, 0.3f, 0.1f, 1.0f);
	static Float4 Olive = Float4(0.5f, 0.5f, 0.0f, 1.0f);
	static Float4 Teal = Float4(0.0f, 0.5f, 0.5f, 1.0f);

	static Float3 Iron  = Float3(0.56, 0.57, 0.58);
};

