#include "Framework/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    Application app;
    if (!app.Initialize(hInstance)) {
        return -1;
    }
    return app.Run();
}