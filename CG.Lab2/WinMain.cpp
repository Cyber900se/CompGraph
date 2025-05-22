#include "Framework/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    Application app(hInstance);
    if (!app.Initialize()) return -1;
    return app.Run();
}


