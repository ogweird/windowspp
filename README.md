# windowspp
Object oriented single header library made for simplifying creating native windows in C++

# Example usage
```cpp
#include "windowspp.h"

#include <iostream>

int main() {
    // Create WindowsPP instance
    auto App = WindowsPP();

    // Create smart window (smart because smart pointer (get it? (im kms)))
    auto Wnd = App.MakeSmartWindow("Hello!!", 800, 600);

    // Create WindowEvents object for custom callbacks
    WindowEvents events{};

    // Create a callback using a lambda function
    events.Resize = [](int x, int y) {
        std::cout << "New Size: " << x << "x" << y << std::endl;
    };

    // Set the callbacks
    Wnd->SetEvents(events);

    // Run the ting u get me
    return App.Run();
}
```
