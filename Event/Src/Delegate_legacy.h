#pragma once
#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <concepts>

#define BIND_MEM_FN(fn)                                     \
  [this](auto&&... args) -> decltype(auto) {                \
    return this->fn(std::forward<decltype(args)>(args)...); \
  }

/*
options:

use std::function to be type safe;
use vector if the deletion is rare;
map if the deletion is frequent; i.e. dynamic ;

instead of invoke immediately;
use eventBus to manage the events,

*/

namespace Legacy {

// forward declaration;
template <typename Signature>
class FDelegate; 

// partial specialization , accepts a function signature in the form of
// Ret(Args...) this design pattern requires declaration of the type;
template <typename Ret, typename... Args> 
  requires std::invocable<std::function<Ret(Args...)>, Args...> 
  class FDelegate<Ret(Args...)> {
 public:
  using FunctionType = std::function<Ret(Args...)>;

  // register, bind, Connect
  void Connect(FunctionType&& func) { _callBackFn.emplace_back(std::move(func)); }

  //Blocking; dispatch, execute, broadcast, Emit 
  void Broadcast(auto&&... args) {
    for (auto& func : _callBackFn) {
      if (func) {
        func(std::forward<decltype(args)>(args)...);
      }
    }
  }

 private:
  std::vector<FunctionType> _callBackFn;  // slot
};









template <typename Signature>
class FQueueDelegate; 


template <typename Ret, typename... Args>
class FQueueDelegate<Ret(Args...)> : public FDelegate<Ret(Args...)> {
 public:
  using FunctionType = std::function<Ret(Args...)>;

  template <typename... Args>
  void PostBuffer(Args&&... args) {
    _eventQueue.emplace(std::make_tuple(std::forward<Args>(args)...));
  }

  void DispatchBuffer() {
    while (!_eventQueue.empty()) {
      auto& event = _eventQueue.front();
      std::apply(
          [this](Args... args) {
            this->BlockingBroadCast(std::forward<decltype(args)>(args)...);
          },
          event);
      _eventQueue.pop();
    }
  }

 private:
  std::queue<std::tuple<Args...>> _eventQueue;
};


}  // namespace  
