#pragma once

#include <concepts> 
#include <initializer_list>
#include <cassert>

#include <span> 
#include <type_traits>

#include <algorithm>  //copy
#include <iterator>

//out of range
#include <stdexcept>

#include <execution>
#include <numeric>  // For std::transform

//future C++23:  mdspan 

using namespace std;

namespace MMath
{ 
 

	template <typename T, uint32_t Rows>
	class ColView {
	private:
		static constexpr uint32_t m_size = Rows;
		
	public:
		//span has constraints 
		ColView(std::span<T, Rows> _view) noexcept : m_view(_view) {}

	public:
		T& operator()(uint32_t index) { 
			assert(index < m_size);
			if (index >= m_size) {
				throw std::out_of_range("index out of bounds");
			}
			return m_view[index];
		}   

	public:
		// support assignment

		ColView& operator=(const ColView& other) = default; 
		
		ColView& operator=(const std::array<T, Rows>& arr) noexcept {
			std::copy(arr.begin(), arr.end(), m_view.begin());
			return *this;
		} 

		ColView& operator=(const std::span<T, Rows>& arr) noexcept { 
			std::copy(arr.begin(), arr.end(), m_view.begin());
			return *this;
		}   


	public:
		uint32_t size() const { return m_size; }

		std::span<T, Rows>& data() { return m_view; }
		const std::span<T, Rows>& data() const { return m_view; } 

	private:
		std::span<T, Rows> m_view;
	};







	//template <typename T>   requires std::is_floating_point_v<T>
	template<typename T, uint32_t Rows, uint32_t Cols>
	class Matrix
	{  
	private: 
		static_assert(Rows > 0 && Cols > 0, "Matrix must have at least one row and one column"); //before member evaluations
		static constexpr uint32_t m_size = Rows * Cols; 

	public: 
		 Matrix() = default;
	
		 Matrix(std::span<T, m_size> values) noexcept
		 {
			 std::copy(values.begin(), values.end(), m_data);
		 }   

		 // Constructor taking a flat list  
		 template <typename... Args>
			 requires   ((std::convertible_to<Args, T> && ...)) 
		 Matrix(Args... args) noexcept 
		 {
		   static_assert (sizeof...(Args) == m_size, "Number of arguments must match the size of the matrix");
		   auto _temp = { args... };
		   std::copy(_temp.begin(), _temp.end(), m_data);
		 }  


	public: 
		 template<uint32_t index>
		 ColView<T, Rows> col()
	     { 
		 	static_assert(index < Cols, "column index out of bounds");
		     auto _colView = ColView<T, Rows>(std::span<T, Rows>(m_data + index * Rows, Rows));
		     return _colView;
	     } 
		 
		 ColView<T, Rows> col(uint32_t index)
		 {
			 if (index >= m_size) {
				 throw std::out_of_range("index out of bounds");
			 }
			 auto _colView = ColView<T, Rows>(std::span<T, Rows>(m_data + index * Rows, Rows));
			 return _colView;
		 }   


		 T& operator()(uint32_t index) {
			 assert(index < m_size);
			 if (index >= m_size) {
				 throw std::out_of_range("index out of bounds");
			 }
			 return m_data[index];
		 }


		 uint32_t size() const { return m_size; }

		 T* data()  { return &m_data[0]; }
		 const T* data() const { return &m_data[0]; }

		  
	private:
		T m_data[m_size];  
	};



	template <typename T, uint32_t Rows, uint32_t Cols>
	Matrix<T, Rows, Cols> operator+(const Matrix<T, Rows, Cols>& lhs, const Matrix<T, Rows, Cols>& rhs) {

		Matrix<T, Rows, Cols> result{};

		std::transform(std::execution::par_unseq, lhs.data(), lhs.data() + lhs.size(), rhs.data(), result.data(), std::plus<>{}); 
		 
		return result;
	}




	
	template <typename T, uint32_t Rows>
	Matrix<T, Rows, 1> ViewToVector(ColView<T, Rows> col)
	{
		Matrix<T, Rows, 1> vec(col.data());
		return vec;
	}
	//using std::move here is typically redundant in modern C++






	typedef Matrix<float, 2, 2> mat2;
	typedef Matrix<float, 3, 3> mat3;
	typedef Matrix<float, 4, 4> mat4;

	typedef Matrix<float, 2, 1> vec2;
	typedef Matrix<float, 3, 1> vec3; 
	typedef Matrix<float, 4, 1> vec4;

}