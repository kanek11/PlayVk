
#include "Application.h"

#include <array>


#include "Vector.h"
#define TESTMATH 0
#define RunApplication 1

/*
void TestMath()
{ 
	using namespace MMath; 

	//init:
	{
		FLOAT3 v1; 
		FLOAT3 v2({ 1.f, 2.f, 3.f });
		
		std::array<float, 3> stdArr = { 1.f, 2.f, 3.f };
		FLOAT3 v3(stdArr);  

		//c-array:
		float c_array[3] = { 1.f, 2.f, 3.f };
		FLOAT3 v4(c_array);    
 
	}

	//access channels
	{
		FLOAT3 v1({ 1.f, 2.f, 3.f });
		assert(v1[0] == 1.f && v1[1] == 2.f && v1[2] == 3.f);
		assert(v1.x() == 1.f && v1.y() == 2.f && v1.z() == 3.f);

		FLOAT2 v2 = v1.xy();   
		assert(v2[0] == 1.f && v2[1] == 2.f);
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

	}


	std::cout << "Math tests passed!" << std::endl; 
} 

*/


int main(int argc, char** argv)
{  
#if defined(TESTMATH)
	//TestMath();
#endif


#if defined(RunApplication)
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