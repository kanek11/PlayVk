#pragma once 
#include <memory>

// basic macros;
/*
member function has a implicit parameter, this;
the best solution for now is to use generic lambda to wrap it;
*/ 
#define BIND_MEM_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
 
//variant helper
//template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
//template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


// see assert
#define RD_EXPAND_MACRO(x) x
#define RD_STRINGIFY_MACRO(x) #x



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

// weak reference, no ownership
template <typename T>
using WeakPtr = std::weak_ptr<T>;
template <typename T, typename... Args>
constexpr WeakPtr<T> CreateWeak(std::shared_ptr<T> sharedPtr)
{
    return WeakPtr<T>(sharedPtr);
}
