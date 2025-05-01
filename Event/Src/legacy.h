#pragma once


   void Unregister(size_t id) {
  if (id < _callBacks.size()) {
    _callBacks[id]._func = nullptr;  // ���ʱ���ص�����Ϊ nullptr
  }
}


// ����ص�����Ľṹ��
class DelegateHandle {
 public:
  DelegateHandle(FDelegate& delegate, size_t id)
      : _delegate(delegate), _id(id) {}

  ~DelegateHandle() { _delegate.Unregister(_id); }

  DelegateHandle(const DelegateHandle&) = delete;
  DelegateHandle& operator=(const DelegateHandle&) = delete;

 private:
  FDelegate& _delegate;
  size_t _id;
};

// register, bind, Connect
[[nodiscard]] std::shared_ptr<DelegateHandle> Connect(FunctionType func,
                                                      OneShotTag) {
  _callBacks.emplace_back(std::move(func), OneShotTag{});
  return std::make_shared<DelegateHandle>(*this, _callBacks.size() - 1);
}

[[nodiscard]] std::shared_ptr<DelegateHandle> Connect(FunctionType func,
                                                      PersistentTag) {
  _callBacks.emplace_back(std::move(func), PersistentTag{});
  return std::make_shared<DelegateHandle>(*this, _callBacks.size() - 1);
}