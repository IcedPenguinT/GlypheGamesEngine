#include "Platform.h"

#if KPLATFORM_WINDOWS

#include "core/Logger.h"
#include "core/Input.h"
#include "containers/Darray.h"

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>


typedef struct InternalState {
    HINSTANCE hInstance;
    HWND hwnd;
} InternalState;

static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);

b8 PlatformStartup(
    PlatformState* platState,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height
) {
    platState->InternalState = malloc(sizeof(InternalState));
    InternalState *state = (InternalState *)platState->InternalState;

    state->hInstance = GetModuleHandleA(0);

    // Setup and register window class
    HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = Win32ProcessMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "GlypheGames";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    //Create window
    u32 clientX = x;
    u32 clientY = y;
    u32 clientWidth = width;
    u32 clientHeight = height;
    
    u32 windowX = clientX;
    u32 windowY = clientY;
    u32 windowWidth = clientWidth;
    u32 windowHeight = clientHeight;

    u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 windowExStyle = WS_EX_APPWINDOW;

    windowStyle |= WS_MAXIMIZEBOX;
    windowStyle |= WS_MINIMIZEBOX;
    windowStyle |= WS_THICKFRAME;

    RECT borderRect = {0, 0, 0, 0};
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

    windowX += borderRect.left;
    windowY += borderRect.top;

    windowWidth += borderRect.right - borderRect.left;
    windowHeight += borderRect.bottom - borderRect.top;

    HWND handle = CreateWindowExA(
        windowExStyle, "GlypheGames", applicationName,
        windowStyle, windowX, windowY, windowWidth, windowHeight,
        0, 0, state->hInstance, 0
    );

    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        KFATAL("Window creation failed!");
        return FALSE;
    } else {
        state->hwnd = handle;
    }

    b32 shouldActivate = 1; //TODO: This should be false.
    i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(state->hwnd, showWindowCommandFlags);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceFrequency(&startTime);

    return TRUE;
}

void PlatformShutdown(PlatformState *platState){
    InternalState *state = (InternalState *)platState->InternalState;

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 PlatformPumpMessages(PlatformState* platState) {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* PlatformAllocate(u64 size, b8 aligned) {
    return malloc(size);
}

void PlatformFree(void* block, b8 aligned) {
    free(block);
}

void* PlatformZeroMemory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* PlatformCopyMemory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* PlatformSetMemory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void PlatformConsoleWrite(const char* message, u8 colour) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(consoleHandle, levels[colour]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, numberWritten, 0);
}

void PlatformConsoleWriteError(const char* message, u8 colour) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(consoleHandle, levels[colour]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, numberWritten, 0);
}

f64 PlatformGetAbsoluteTime() {
    LARGE_INTEGER nowTime;
    QueryPerformanceCounter(&nowTime);
    return (f64)nowTime.QuadPart * clockFrequency;
}

void PlatformSleep(u64 ms) {
    Sleep(ms);
}

void PlatformGetRequiredExtensionNames(const char*** namesDarray){
    DarrayPush(*namesDarray, &"VK_KHR_win32_surface");
}

LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam) {
    switch(msg){
        case WM_ERASEBKGND:

            return 1;
        case WM_CLOSE:
            // TODO: Fire event for the application to quit.
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            //RECT r;
            //GetCLientRect(hwnd, &r);
            //u32 width = r.right -r.left;
            //u32 height = r.bottom - r.top;

            // TODO: Fire event for window resize;
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            Keys key = (u16)wParam;

            InputProcessKey(key, pressed);
        } break;
        case WM_MOUSEMOVE: {
            i32 xPosition = GET_X_LPARAM(lParam);
            i32 yPosition = GET_Y_LPARAM(lParam);
            
            InputProcessMouseMove(xPosition, yPosition);
        }
        case WM_MOUSEWHEEL: {
            i32 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0) {
                zDelta = (zDelta < 0) ? -1 : 1;

                InputProcessMouseWheel(zDelta);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            
            Buttons mouseButton = BUTTON_MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouseButton = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouseButton = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouseButton = BUTTON_RIGHT;
                    break;
            }

            if (mouseButton != BUTTON_MAX_BUTTONS)
                InputProcessButton(mouseButton, pressed);
        } break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif //KPLATFORM_WINDOWS