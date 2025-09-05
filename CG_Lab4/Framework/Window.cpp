#include "Window.h"

static Keys MapVkToKeys(WPARAM wparam, LPARAM lparam)
{
    switch (wparam)
    {
        case VK_SHIFT:
        {
            // Для SHIFT у WM_KEYDOWN в wparam всегда VK_SHIFT,
            // различаем левый/правый по scan code
            const UINT sc = (lparam >> 16) & 0xFF;
            const UINT vkEx = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);
            return (vkEx == VK_RSHIFT) ? Keys::RightShift : Keys::LeftShift;
        }
        case VK_CONTROL:
            // Бит 24 lParam — «extended»: правые Ctrl/Alt помечены extended
            return (lparam & (1u << 24)) ? Keys::RightControl : Keys::LeftControl;
        case VK_MENU: // Alt
            return (lparam & (1u << 24)) ? Keys::RightAlt : Keys::LeftAlt;
        default:
            return static_cast<Keys>(wparam);
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    Window* pThis = nullptr;
    if (umessage == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCT*>(lparam);
        pThis = static_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (umessage) {
            case WM_KEYDOWN:
                if (!(lparam & (1 << 30))) // подавляем авто-повтор
                    pThis->inputHandler.SetKeyDown(MapVkToKeys(wparam, lparam));
                return 0;

            case WM_KEYUP:
                pThis->inputHandler.SetKeyUp(MapVkToKeys(wparam, lparam));
                return 0;

            case WM_SYSKEYDOWN:
                if (!(lparam & (1 << 30)))
                    pThis->inputHandler.SetKeyDown(MapVkToKeys(wparam, lparam));
                return 0;

            case WM_SYSKEYUP:
                pThis->inputHandler.SetKeyUp(MapVkToKeys(wparam, lparam));
                return 0;

            case WM_MOUSEMOVE: {
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                pThis->inputHandler.SetMousePosition(x, y);
                break;
            }

            case WM_LBUTTONDOWN:
                pThis->inputHandler.SetMouseButtonDown(Keys::LeftButton);
                return 0;
            case WM_LBUTTONUP:
                pThis->inputHandler.SetMouseButtonUp(Keys::LeftButton);
                return 0;

            case WM_RBUTTONDOWN:
                pThis->inputHandler.SetMouseButtonDown(Keys::RightButton);
                return 0;
            case WM_RBUTTONUP:
                pThis->inputHandler.SetMouseButtonUp(Keys::RightButton);
                return 0;

            case WM_MBUTTONDOWN:
                pThis->inputHandler.SetMouseButtonDown(Keys::MiddleButton);
                return 0;
            case WM_MBUTTONUP:
                pThis->inputHandler.SetMouseButtonUp(Keys::MiddleButton);
                return 0;

            case WM_XBUTTONDOWN:
                if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
                    pThis->inputHandler.SetMouseButtonDown(Keys::MouseButtonX1);
                } else if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                    pThis->inputHandler.SetMouseButtonDown(Keys::MouseButtonX2);
                }
                return 0;

            case WM_XBUTTONUP:
                if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
                    pThis->inputHandler.SetMouseButtonUp(Keys::MouseButtonX1);
                } else if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                    pThis->inputHandler.SetMouseButtonUp(Keys::MouseButtonX2);
                }
                return 0;

            case WM_MOUSEWHEEL: {
                int delta = GET_WHEEL_DELTA_WPARAM(wparam);
                pThis->inputHandler.AddMouseWheelDelta(delta);
                return 0;
            }

            case WM_SIZE: {
                UINT width = LOWORD(lparam);
                UINT height = HIWORD(lparam);
                pThis->SetSize(width, height);
                pThis->sizeChanged = true;
                return 0;
            }

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            default: ;
        }
    }

    return DefWindowProc(hwnd, umessage, wparam, lparam);
}


Window::Window(HINSTANCE hInst, const wchar_t* applicationName, UINT width, UINT height)
    : hInstance(hInst) {

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = applicationName;

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"RegisterClassEx failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

    auto posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    auto posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
    dwStyle,
    posX, posY,
    windowRect.right - windowRect.left,
    windowRect.bottom - windowRect.top,
    nullptr, nullptr, hInstance, this);

    if (!hwnd) {
        MessageBox(nullptr, L"CreateWindowEx failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    ShowCursor(true);
}

Window::~Window() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

void Window::SetSize(UINT width, UINT height) {
    this->width = width;
    this->height = height;
}

bool Window::CheckSizeChanged() {
    bool changed = sizeChanged;
    sizeChanged = false;
    return changed;
}

bool Window::ProcessMessages() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (msg.message == WM_QUIT)
        return true;
    return false;
}