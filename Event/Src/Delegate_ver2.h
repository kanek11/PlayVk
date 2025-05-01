#pragma once


#include <functional>
#include <vector>
#include <memory> 
#include <concepts>
#include <variant> 
#include <algorithm>  

//todo ,the unregister scheme;

// Define compile-time tags for callback types
struct PersistentTag {};
struct OneShotTag {};
//struct ConditionalTag {};  // For future extension if needed



template <typename Signature>
class FDelegate;


template <typename Ret, typename... Args>
  requires std::invocable<std::function<Ret(Args...)>, Args...>
class FDelegate<Ret(Args...)> {
 public:
  using FunctionType = std::function<Ret(Args...)>;

  struct CallBackEntry {
    FunctionType _func;
    std::variant<PersistentTag, OneShotTag> _tag; 
  };

      // 管理回调句柄的结构体
  class DelegateHandle {
   public:
    DelegateHandle(FDelegate& delegate, size_t id)
        : _delegate(delegate), _id(id) {}

    ~DelegateHandle() { 
      _delegate.Unregister(_id);
    }

    DelegateHandle(const DelegateHandle&) = delete;
    DelegateHandle& operator=(const DelegateHandle&) = delete;

   private:
    FDelegate& _delegate;
    size_t _id;
  };


  // register, bind, Connect
  [[nodiscard]] std::shared_ptr<DelegateHandle> Connect(FunctionType func, OneShotTag) {
    _callBacks.emplace_back(std::move(func), OneShotTag{});
    return std::make_shared<DelegateHandle>(*this, _nextId++);
  }

  [[nodiscard]] std::shared_ptr<DelegateHandle> Connect(FunctionType func, PersistentTag) {
    _callBacks.emplace_back(std::move(func), PersistentTag{});
    return std::make_shared<DelegateHandle>(*this, _nextId++);
  } 

  // Blocking; dispatch, execute, broadcast, Emit
  void Broadcast(auto&&... args) {

      for (auto entry = _callBacks.begin(); entry != _callBacks.end();) {
      // Visit the `tag` member specifically, not the entire CallbackEntry
      std::visit(
          [&](auto&& tag) {
            using Tag = std::decay_t<decltype(tag)>;

            if constexpr (std::is_same_v<Tag, PersistentTag>) { 
              entry->_func(std::forward<decltype(args)>(args)...);
            } else if constexpr (std::is_same_v<Tag, OneShotTag>) { 
              entry->_func(std::forward<decltype(args)>(args)...);
              entry->_func = nullptr;   
            }
          },
          entry->_tag);  // Visit only the `type` variant
      ++entry;
    }

      // todo: this can be optimized
     _callBacks.erase( std::remove_if(_callBacks.begin(), _callBacks.end(),
                       [](const auto& entry) { return entry._func == nullptr; }), _callBacks.end());

  }

   void Unregister(size_t id) { 
       // todo: garbage collection like aways;
  }

   size_t CallBackNums() const { return _callBacks.size(); }
  
private: 
  std::vector<CallBackEntry> _callBacks;  // slot
  size_t _nextId = 0;
};
