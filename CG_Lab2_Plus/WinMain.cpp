#include "Framework/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    Application app;
    if (!app.Initialize(hInstance)) {
        return -1;
    }
    return app.Run();
}