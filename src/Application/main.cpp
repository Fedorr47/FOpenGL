
#include "Application/GLApplication.h"
#include <iostream>

int main()
{
    try {
        GLApplication app(1280, 720, "FOpenGL");
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
