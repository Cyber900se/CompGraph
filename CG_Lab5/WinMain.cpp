#include "Framework/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    srand(static_cast<unsigned>(time(nullptr)));
    Application app;
    if (!app.Initialize(hInstance)) {
        return -1;
    }
    return app.Run();
}