#include "core/Input.h"
#include "core/Event.h"
#include "core/Memory.h"
#include "core/Logger.h"

typedef struct KeyboardState {
    b8 keys[256];
} KeyboardState;

typedef struct MouseState {
    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
} MouseState;

typedef struct InputState {
    KeyboardState keyboardCurrent;
    KeyboardState keyboardPrevious;
    MouseState mouseCurrent;
    MouseState mousePrevious;
} InputState;

static InputState* statePtr;

void InputSystemInitialize(u64* memoryRequirements, void* state) {
    *memoryRequirements = sizeof(InputState);
    if (state == 0) {
        return;
    }
    ZeroMemory(state, sizeof(InputState));
    statePtr = state;
    KINFO("Input subsystem initialized.");
}

void InputSystemShutdown() {
    statePtr = 0;
}

void InputUpdate(f64 deltaTime) {
    if (!statePtr)
        return;
    
    CopyMemory(&statePtr->keyboardPrevious, &statePtr->keyboardCurrent, sizeof(KeyboardState));
    CopyMemory(&statePtr->mousePrevious, &statePtr->mouseCurrent, sizeof(MouseState));
}

b8 InputIsKeyDown(Keys key) {
    if (!statePtr)
        return false;
    return statePtr->keyboardCurrent.keys[key] == true;
}

b8 InputIsKeyUp(Keys key) {
    if (!statePtr)
        return false;
    return statePtr->keyboardCurrent.keys[key] == false;
}

b8 InputWasKeyDown(Keys key) {
    if (!statePtr)
        return false;
    return statePtr->keyboardPrevious.keys[key] == true;
}

b8 InputWasKeyUp(Keys key) {
    if (!statePtr)
        return false;
    return statePtr->keyboardPrevious.keys[key] == false;
}

void InputProcessKey(Keys key, b8 pressed) {
    if (statePtr && statePtr->keyboardCurrent.keys[key] != pressed)
        statePtr->keyboardCurrent.keys[key] = pressed;

    if (key == KEY_LALT) {
        KINFO("Left alt %s.", pressed ? "pressed" : "released");
    }
    else if (key == KEY_RALT)
        KINFO("Right alt %s.", pressed ? "pressed" : "released");

    if (key == KEY_LCONTROL) {
        KINFO("Left ctrl %s.", pressed ? "pressed" : "released");
    }
    else if (key == KEY_RCONTROL)
        KINFO("Right ctrl %s.", pressed ? "pressed" : "released");

    if (key == KEY_LSHIFT) {
        KINFO("Left shift %s.", pressed ? "pressed" : "released");
    }
    else if (key == KEY_RSHIFT)
        KINFO("Right shift %s.", pressed ? "pressed" : "released");
    
    if (statePtr->keyboardCurrent.keys[key] != pressed) {
        statePtr->keyboardCurrent.keys[key] = pressed;
        
        EventContext context;
        context.Data.u16[0] = key;
        EventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

b8 InputIsButtonDown(Buttons button) {
    if (!statePtr)
        return false;
    return statePtr->mouseCurrent.buttons[button] == true;
}

b8 InputIsButtonUp(Buttons button) {
    if (!statePtr)
        return false;
    return statePtr->mouseCurrent.buttons[button] == false;
}

b8 InputWasButtonDown(Buttons button) {
    if (!statePtr)
        return false;
    return statePtr->mousePrevious.buttons[button] == true;
}

b8 InputWasButtonUp(Buttons button) {
    if (!statePtr)
        return false;
    return statePtr->mousePrevious.buttons[button] == false;
}

void InputGetMousePosition(i32* x, i32* y) {
    if (!statePtr) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = statePtr->mouseCurrent.x;
    *y = statePtr->mouseCurrent.y;
}

void InputGetPreviousMousePosition(i32* x, i32* y) {
    if (!statePtr) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = statePtr->mousePrevious.x;
    *y = statePtr->mousePrevious.y;
}

void InputProcessButton(Buttons button, b8 pressed) {
    if (statePtr->mouseCurrent.buttons[button] != pressed) {
        statePtr->mouseCurrent.buttons[button] = pressed;

        EventContext context;
        context.Data.u16[0] = button;
        EventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void InputProcessMouseMove(i16 x, i16 y) {
    if (statePtr->mouseCurrent.x != x || statePtr->mouseCurrent.y != y) {
        // NOTE: Enable this line if debugging.
        //KDEBUG("Mouse pos: %i, %i!", x, y);

        statePtr->mouseCurrent.x = x;
        statePtr->mouseCurrent.y = y;

        EventContext context;
        context.Data.u16[0] = x;
        context.Data.u16[1] = y;
        EventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void InputProcessMouseWheel(i8 zDelta) {
    EventContext context;
    context.Data.u8[0] = zDelta;
    EventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}