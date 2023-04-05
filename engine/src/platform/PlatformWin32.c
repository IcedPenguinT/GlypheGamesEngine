#include "Platform.h"

#if KPLATFORM_WINDOWS

#include "core/Logger.h"
#include "core/Input.h"
#include "core/Event.h"
#include "containers/Darray.h"

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/VulkanTypes.inl"


typedef struct PlatformState {
    HINSTANCE hInstance;
    HWND hwnd;
    VkSurfaceKHR surface;
} PlatformState;

static PlatformState *statePtr;
static f64 clockFrequency;
static LARGE_INTEGER startTime;


LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);

void ClockSetup() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);
}

b8 PlatformSystemStartup(
    u64 *memoryRequirement,
    void *state,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height
) {
    *memoryRequirement = sizeof(PlatformState);
    if (state == 0) {
        return true;
    }
    statePtr = state;
    statePtr->hInstance = GetModuleHandleA(0);


    // Setup and register window class
    HICON icon = LoadIcon(statePtr->hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = Win32ProcessMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = statePtr->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "GlypheGames";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
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
        0, 0, statePtr->hInstance, 0
    );

    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        KFATAL("Window creation failed!");
        return false;
    } else {
        statePtr->hwnd = handle;
    }

    b32 shouldActivate = 1; //TODO: This should be false.
    i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(statePtr->hwnd, showWindowCommandFlags);

    ClockSetup();

    return true;
}

void PlatformSystemShutdown(void *platState){
    if (statePtr && statePtr->hwnd) {
        DestroyWindow(statePtr->hwnd);
        statePtr->hwnd = 0;
    }
}

b8 PlatformPumpMessages() {
    if (statePtr) {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

    return true;
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
    if (!clockFrequency)
        ClockSetup();

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

b8 PlatformCreateVulkanSurface(VulkanContext *context) {
    if (!statePtr) 
        return false;
    VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    createInfo.hinstance = statePtr->hInstance;
    createInfo.hwnd = statePtr->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &createInfo, context->allocator, &statePtr->surface);
    if (result != VK_SUCCESS) {
        KFATAL("Vulkan surface creation failed.");
        return false;
    }

    context->surface = statePtr->surface;
    return true;

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
            RECT r;
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;

            // Fire the event. The application layer should pick this up, but not handle it
            // as it shouldn be visible to other parts of the application.
            EventContext context;
            context.Data.u16[0] = (u16)width;
            context.Data.u16[1] = (u16)height;
            EventFire(EVENT_CODE_RESIZED, 0, context);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            Keys key = (u16)wParam;

            b8 isExtended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

            if (wParam == VK_MENU) {
                key = isExtended ? KEY_RALT : KEY_LALT;
            }
            else if (wParam == VK_SHIFT) {
                u32 leftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                u32 scancode = ((lParam & (0xFF << 16)) >> 16);
                key = scancode == leftShift ? KEY_LSHIFT : KEY_RSHIFT;
            }
            else if (wParam == VK_CONTROL) {
                key = isExtended ? KEY_RCONTROL : KEY_LCONTROL;
            }
            
            InputProcessKey(key, pressed);
            return 0;
        }
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