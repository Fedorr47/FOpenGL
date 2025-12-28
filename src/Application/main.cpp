
#include "Application/GLApplication.h"
#include <iostream>

#define GLEW_STATIC

#ifdef _MSC_VER
  #include <crtdbg.h>
  #define _CRTDBG_MAP_ALLOC
#endif

int main()
{
#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(N); // чтобы остановиться на конкретной утечке
#endif
    try {
        GLApplication app(1280, 720, "FOpenGL");
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
