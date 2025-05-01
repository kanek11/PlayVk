#pragma once


template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;


Matrix(std::initializer_list<T> values)
{
	assert(values.size() == m_size);
	if (values.size() != m_size) {
		throw std::invalid_argument("Invalid number of elements");
	}
	std::copy(values.begin(), values.end(), m_data);
}



template <typename T, uint32_t Rows>
Matrix<T, Rows, 1> ToVector(ColView<T, Rows> col)
{
	Matrix<T, Rows, 1> vec(col);
	return vec;
}


Matrix(ColView <T, Rows> col) noexcept
{
	std::copy(col.data().begin(), col.data().end(), m_data);
}




template <typename T>
concept MatrixLike = requires(T a) {
	{ a.size() } -> std::convertible_to<std::uint32_t>;
	{ a.data() } -> std::same_as<decltype(a.data())>;  // Ensures `data()` exists 
};

//template <typename T, uint32_t Rows, uint32_t Cols>
//Matrix<T, Rows, Cols> operator+(const Matrix<T, Rows, Cols>& lhs, const Matrix<T, Rows, Cols>& rhs) {
//	Matrix<T, Rows, Cols> result;
//
//	//modern C++ way to add to array:
//	std::transform(lhs.data, lhs.data + Rows * Cols, rhs.data, result.data, std::plus<T>()); 
//	 
//	return result;
//}

template <MatrixLike MatrixType>
MatrixType operator+(const MatrixType& lhs, const MatrixType& rhs) {

	MatrixType result{};

	auto lhs_data = lhs.data();
	auto rhs_data = rhs.data();
	auto result_data = result.data();


	for (uint32_t i = 0; i < lhs.size(); ++i) {
		result_data[i] = lhs_data[i] + rhs_data[i];
	}
	return result;
}





std::begin(lhs.data()),
std::end(lhs.data()),
std::begin(rhs.data()),
std::begin(result.data()),
std::plus<>{});

//T& operator()(uint32_t index) {
   // if (index >= size) {
   //	 throw std::out_of_range("index out of bounds");
   // }
   // return data[index];
//}
template<uint32_t index>
T& get() {
	static_assert(index < size, "index out of bounds");
	return view[index];
}


template<uint32_t index>
ColView col()
{
	static_assert(index < Cols, "column index out of bounds");
	auto _colView = ColView(data + index * Rows);
	return _colView;
}
eigen官方的这种写法下.block()  在你看来，i mean modern container design看来 会不会在readonly？copy？reference？这种写法会不会比较模糊#include <Eigen/Dense>
#include <iostream> 
using namespace std;
int main()
{
	Eigen::MatrixXf m(3, 3);
	m << 1, 2, 3,
		4, 5, 6,
		7, 8, 9;
	cout << "Here is the matrix m:" << endl << m << endl;
	cout << "2nd Row: " << m.row(1) << endl;
	m.col(2) += 3 * m.col(0);
	cout << "After adding 3 times the first column into the third column, the matrix m is:\n";
	cout << m << endl;
}


template <uint32_t SubRows, uint32_t SubCols>
Matrix<T, SubRows, SubCols> block(uint32_t startRow, uint32_t startCol) const {


	Matrix<T, SubRows, SubCols> submatrix;

	for (std::size_t i = 0; i < SubRows; ++i) {
		for (std::size_t j = 0; j < SubCols; ++j) {
			submatrix.data[i * SubCols + j] = data[(startRow + i) * Cols + (startCol + j)];
		}
	}
	return submatrix;
}

template<uint32_t index>
Col_type col() const
{
	static_assert(index < Cols, "column index out of bounds");
	Col_type temp{};
	for (uint32_t i = 0; i < Rows; ++i)
	{
		temp.data[i] = data[i * Cols + index];
	}
	return temp;
}

template<uint32_t index>
Row_type row() const
{
	static_assert(index < Rows, "row index out of bounds");
	Row_type temp{};

	std::copy(
		std::begin(data) + index * Cols,
		std::begin(data) + (index + 1) * Cols,
		std::begin(temp.data)
	);

	return temp;
}