#pragma once
#include <memory>
#include <functional>
#include <type_traits>

#include <vulkan/vulkan.h>

/* todo: centralize and keep track?*/
/*
* [typeid,  vector< weak_ptr>]?
*/

template <typename Handle_t>
struct VkHandle {
  Handle_t handle; 
}; 
 
template <typename Handle_t>
std::shared_ptr<VkHandle<Handle_t>> CreateSharedVkHandle(
    Handle_t handle, std::function<void(Handle_t)> dtor) {
    
  auto underlyingDeleter = [dtor](VkHandle<Handle_t>* wrapper) { dtor(wrapper->handle); };

  return std::shared_ptr<VkHandle<Handle_t>>(new VkHandle<Handle_t>{handle},
                                              underlyingDeleter);
}



template <typename Handle_t>
using SharedVkHandle = std::shared_ptr<VkHandle<Handle_t>>;



template<typename T, typename F>
std::vector<T> EnumerateVector(F&& enumerate) {
	uint32_t count = 0;
	enumerate(&count, nullptr);
	std::vector<T> result(count);
	enumerate(&count, result.data());
	return result;
}

