#pragma once
#pragma once
#include <memory> 
#include <stdexcept>
#include <type_traits>
#include <functional>


//vk depth is in range [0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


// basic macros;

// see assert
#define RD_EXPAND_MACRO(x) x
#define RD_STRINGIFY_MACRO(x) #x


/*
member function has a implicit parameter, this;
the best solution for now is to use generic lambda to wrap it;
*/

#define BIND_MEM_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }


//fast integer ceil trick 
template <std::integral T>
T ceil_div(T a, T b) {
    if (b == 0)  throw std::domain_error("Division by zero");

    T result = a / b;
    if (T remainder = a % b; remainder != 0 && ((a > 0) == (b > 0)))
    {
        result += 1;
    }
    return result;
}




 

// sole owner
template <typename T>
using UniquePtr = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr UniquePtr<T> CreateUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// shared owner
template <typename T>
using SharedPtr = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr SharedPtr<T> CreateShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// weak reference no ownership
template <typename T>
using WeakPtr = std::weak_ptr<T>;
template <typename T, typename... Args>
constexpr WeakPtr<T> CreateWeak(std::shared_ptr<T> sharedPtr)
{
    return WeakPtr<T>(sharedPtr);
}
 

