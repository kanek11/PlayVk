#include "PCH.h"
#include "Application.h" 

#include "Math/MMath.h"

#define TESTMATH 0
#define RunApplication 1


void TestMath()
{  
	using FLOAT2 = MMath::FLOAT2;
	using FLOAT3 = MMath::FLOAT3;
	using FLOAT4 = MMath::FLOAT4;

	using FLOAT2X2 = MMath::FLOAT2X2;
	using FLOAT3X3 = MMath::FLOAT3X3;
	using FLOAT4X4 = MMath::FLOAT4X4;

	//init:
	{
		FLOAT3 v1; 
		FLOAT3 v2({ 1.f, 2.f, 3.f });
		
		std::array<float, 3> stdArr = { 1.f, 2.f, 3.f };
		FLOAT3 v3(stdArr);  

		//c-array:
		float c_array[3] = { 1.f, 2.f, 3.f };
		FLOAT3 v4(c_array);    


		FLOAT3 v5( 1.f, 2.f, 3.f ); // uniform initialization)
		assert(v1[0] == 0.f && v1[1] == 0.f && v1[2] == 0.f); // default init 
	}

	//access 
	{
		FLOAT3 v1({ 1.f, 2.f, 3.f });
		assert(v1[0] == 1.f && v1[1] == 2.f && v1[2] == 3.f);
		assert(v1.x() == 1.f && v1.y() == 2.f && v1.z() == 3.f);

		assert(v1.get<0>() == 1.f && v1.get<1>() == 2.f && v1.get<2>() == 3.f);  

		//v1.get<4>();  //fail at compiling; static assertion
		//v1[4];  //fail at execution;
		//v1.w(); //fail at compiling;

		FLOAT2 v2 = v1.xy();   
		assert(v2[0] == 1.f && v2[1] == 2.f);

		
	}


	//structural binding
	{
		struct AABB
		{
			FLOAT3 min;
			FLOAT3 max;
		};

		AABB box = { {1.f, 2.f, 3.f}, {4.f, 5.f, 6.f} };
		auto [min, max] = box;

		auto [xMin, yMin, zMin] = min;

		assert(min[0] == 1.f && min[1] == 2.f && min[2] == 3.f);
		assert(max[0] == 4.f && max[1] == 5.f && max[2] == 6.f);
		assert(xMin == 1.f && yMin == 2.f && zMin == 3.f); // Check if structured binding works correctly


		FLOAT3 v1 = { 1.f, 2.f, 3.f };
		auto [x, y, z] = v1;
		assert(x == 1.f && y == 2.f && z == 3.f); // Check if structured binding works correctly

	}

 
	//copy-move behavior
	{
		FLOAT3 v1({ 1.f, 2.f, 3.f });
		FLOAT3 v2 = v1; 
		FLOAT3 v3 = std::move(v1); 
		assert(v2[0] == 1.f && v2[1] == 2.f && v2[2] == 3.f);
		assert(v3[0] == 1.f && v3[1] == 2.f && v3[2] == 3.f);

		v3 = v2;  // assignment
		assert(v3[0] == 1.f && v3[1] == 2.f && v3[2] == 3.f);

		v3 = std::move(v2);  // move assignment
		assert(v3[0] == 1.f && v3[1] == 2.f && v3[2] == 3.f);
	}

	{
		FLOAT3 v1 = { 1.f, 2.f, 3.f };
		FLOAT3 v2 = { 4.f, 5.f, 6.f };
		auto dotResult = Dot(v1, v2);
		assert(dotResult == 1 * 4 + 2 * 5 + 3 * 6);
	}

	//math operations:
	{
		FLOAT3 v1 = { 1.f, 2.f, 3.f };
		FLOAT3 v2 = { 4.f, 5.f, 6.f };
		auto v3 = v1 + v2;  // Vector addition

		assert(v3[0] == 5.f && v3[1] == 7.f && v3[2] == 9.f);
		auto v4 = v1 - v2;  // Vector subtraction
		assert(v4[0] == -3.f && v4[1] == -3.f && v4[2] == -3.f);

		auto v5 = v1 * 2.f;  // Scalar multiplication
		assert(v5[0] == 2.f && v5[1] == 4.f && v5[2] == 6.f);

		auto v6 = v1 / 2.f;  // Scalar division
		assert(v6[0] == 0.5f && v6[1] == 1.f && v6[2] == 1.5f);


		//equality:
		FLOAT3 v7 = { 1.f, 2.f, 3.f };
		FLOAT3 v8 = { 1.f, 2.f, 3.f };
		FLOAT3 v9 = { 4.f, 5.f, 6.f };
		assert(v7 == v8);   
		assert(!(v7 == v9));  

	}

	//normalize and length

	{
		FLOAT3 v1 = { 3.f, 4.f, 0.f };
		auto length = Length(v1);
		assert(length == 5.f);  

		auto normalized = Normalize(v1); 
		assert((Length(normalized) - 1.0f) < 1e-6f); // Check if normalized length is close to 1

	}



	//matrix 
	//matrix init:
	{
		FLOAT3X3 mat1 =
		{
			FLOAT3{1.f, 2.f, 3.f},
			FLOAT3{4.f, 5.f, 6.f},
			FLOAT3{7.f, 8.f, 9.f}
		};

		assert(mat1[0][0] == 1.f && mat1[0][1] == 2.f && mat1[0][2] == 3.f);

		FLOAT3X3 mat2 = {
			{1.f, 2.f, 3.f},
			{4.f, 5.f, 6.f},
			{7.f, 8.f, 9.f}
		};

		assert(mat2[0][0] == 1.f && mat2[0][1] == 2.f && mat2[0][2] == 3.f); 
		 
		 
		auto mat3 = MMath::IdentityMatrix<float, 3>();

		auto col0 = FLOAT3{ 1.f, 2.f, 3.f };
		auto col1 = FLOAT3{ 4.f, 5.f, 6.f };
		auto col2 = FLOAT3{ 7.f, 8.f, 9.f };

		//overwrite mat3:
		mat3[0] = col0;
		mat3[1] = col1;
		mat3[2] = col2;

		assert(mat3[0][0] == 1.f && mat3[0][1] == 2.f && mat3[0][2] == 3.f);
		assert(mat3[1][0] == 4.f && mat3[1][1] == 5.f && mat3[1][2] == 6.f);
		assert(mat3[2][0] == 7.f && mat3[2][1] == 8.f && mat3[2][2] == 9.f);
		 
		 

	}


	//matrix column access:
	{
		FLOAT3X3 mat1 = {
			{1.f, 2.f, 3.f},
			{4.f, 5.f, 6.f},
			{7.f, 8.f, 9.f}
		}; 

		// read access first column
		auto& col0 = mat1[0]; 
		assert(col0[0] == 1.f && col0[1] == 2.f && col0[2] == 3.f); 


		// Modify first column
		mat1[0] = { 10.f, 20.f, 30.f }; 
		assert(mat1[0][0] == 10.f && mat1[0][1] == 20.f && mat1[0][2] == 30.f);
		 

		mat1[0] = FLOAT3{ 10.f, 20.f, 30.f };  
		assert(mat1[0][0] == 10.f && mat1[0][1] == 20.f && mat1[0][2] == 30.f);

	}


	//matrix operation
	{
		// Matrix-Vector multiplication
		FLOAT3X3 mat1 = { {1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}, {7.f, 8.f, 9.f} };
		FLOAT3 vec1 = { 1.f, 2.f, 3.f };
		auto resultVec = mat1 * vec1;
		assert(resultVec[0] == 14.f && resultVec[1] == 32.f && resultVec[2] == 50.f);

		//Matrix-Matrix multiplication
		FLOAT4X4 mat2 = { {1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f} };
		FLOAT4X4 mat3 = { {1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}, {13.f, 14.f, 15.f, 16.f} };
		auto resultMat = mat2 * mat3;
		assert(resultMat[0][0] == 1.f && resultMat[0][1] == 2.f && resultMat[0][2] == 3.f && resultMat[0][3] == 4.f);

	}


	std::cout << "Math tests passed!" << std::endl; 
} 




int main(int argc, char** argv)
{  
#if TESTMATH
	TestMath();
#endif


#if RunApplication
	auto app = std::make_shared<GameApplication>();

	try {
		app->onInit();
		app->run();
		app->onDestroy();
	}
	catch (const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << std::endl;
		 
		return -1;
	} 
#endif

	return 0;
}