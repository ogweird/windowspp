# windowspp
Object oriented single header library made for simplifying creating native windows in C++

# example usage
```cpp
#include "windowspp.h"

#include <iostream>

int main() {
    // Create WindowsPP instance
    auto App = WindowsPP();

    // Create smart window (smart because smart pointer (get it? (im kms)))
    auto Wnd = App.MakeWindow("Hello!!", 800, 600);

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
# current goals
- Implementing more important callbacks such as for WM_PAINT, WM_COMMAND etc.
- Implementing a wrapper for UI creation
