#include "GLApplication.h"

int main() {
    GLApplication app(800, 600, "Main Window");
    if (!app.Initialize()) {
        return -1;
    }

    app.Run();
    return 0;
}