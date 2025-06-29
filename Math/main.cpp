
#include <iostream> 
//#include <Eigen/Dense>

#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>  // For glm::value_ptr
#include <DirectXMath.h>  // For DirectX::XMMATRIX

#include "Matrix.h"

int main()
{ 

	//Eigen::Vector3f vec;           // 3D vector with `float` elements
	//vec << 1.0f, 2.0f, 3.0f;       // Initialization of elements

	//std::cout << "Vector: " << std::endl << vec << std::endl;

	//Eigen::Matrix<float, 3, 3> static_mat;
	//static_mat << 1, 2, 3,
	//	4, 5, 6,
	//	7, 8, 9;

	//std::cout << "Static Matrix: " << std::endl << static_mat << std::endl;
 
	 //Eigen::Matrix<double, 2, 2> b{
	 //  {2, 3 },
	 //  {5, 6 },
	 //};

	 //auto block = b.block<2, 2>(0, 0);

	 //b.block<2, 2>(0, 0) = b.block<2, 2>(0, 0) + b.block<2, 2>(0, 0);

	 //cout << b << endl;


	 //Eigen::Vector2f eVec = { 1, 2 };




	//std::array<int, 3 > arr0 = { 1, 2, 3 };
	//std::array<int, 3 > arr2 = arr0;

	//std::vector<int> vec0 = { 1, 2, 3 };  


	//float c_array[3] = { 1, 2, 3  };


	// glm::mat4 glm_mat = glm::mat4(1.0f);  
	// glm::vec2 glm_vec = glm::vec2(1.0f, 2.0f);

	// glm::mat4 perspective = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	// //glm::mat4 view = glm::lookAt();



	using namespace MMath; 

	float data[4] = { 1, 2, 3, 4 }; 
	Matrix<float, 2, 2> mat ({ 1, 2, 3, 4 });
	Matrix<float, 2, 2> mat2(data);  

	Matrix<float, 2, 2> mat3(1.0f, 2.0f, 3.0f, 5.0f);

	std::array<float, 4> arr = { 1, 2, 3, 4 };
	Matrix<float, 2, 2> mat4(arr);


	auto mat_sum = mat + mat2;

	auto matCopy(mat); 



	//uint32_t indx = 1;

	//auto col0 = mat.col(0);  
	//auto col1 = mat.col<1>();
	//col1 = { 5,6 };    


	//vec2 mVec = { 1,2 };  
	//mVec = ViewToVector(col0) + ViewToVector(col0);



	//for (uint32_t i = 0; i < mat3.size(); i++)
	//{
	//	std::cout << mat3.data()[i] << " ";
	//}
	//std::cout << std::endl;


	//for (uint32_t i = 0; i < col0.size(); ++i)
	//{
	//	std::cout << col0(i) << " ";
	//}
	//std::cout << std::endl; 

	//for (uint32_t i = 0; i < col1.size(); ++i)
	//{
	//	std::cout << col1(i) << " ";
	//}
	//std::cout << std::endl;

	//for (uint32_t i = 0; i < mVec.size(); ++i)
	//{
	//	std::cout << mVec(i) << " ";
	//}
	//std::cout << std::endl;

 



}