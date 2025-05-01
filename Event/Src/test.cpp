#include <iostream>

//#include "Delegate_legacy.h"
#include "Delegate_ver2.h"



int main() {

  FDelegate<void(int)> delegate;

  std::cout << "size of delegate: " << sizeof(delegate) << "\n"; 


  std::cout << "callback nums:" << delegate.CallBackNums() << "\n";
  {
    // Register a persistent callback
    auto persisent_handle = delegate.Connect(
        [](int value) {
          std::cout << "Persistent callback: " << value << std::endl;
        },
        PersistentTag{});

    // Register a one-shot callback
    auto oneShot_handle = delegate.Connect(
        [](int value) {
          std::cout << "One-shot callback: " << value << std::endl;
        },
        OneShotTag{});

    
     std::cout << "callback nums:" << delegate.CallBackNums() << "\n";

    // Broadcast events
    std::cout << "First broadcast:\n";
    delegate.Broadcast(42);

    std::cout << "callback nums:" << delegate.CallBackNums() << "\n";

    std::cout << "\nSecond broadcast:\n";
    delegate.Broadcast(84);

    std::cout << "\nThird broadcast:\n";
    delegate.Broadcast(126);

  }
  
   std::cout << "callback nums:" << delegate.CallBackNums() << "\n";

  return 0;
}