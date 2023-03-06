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

static b8 initialize = FALSE;
static InputState state = {};

void InputInitialize() {
    ZeroMemory(&state, sizeof(InputState));
    initialize = TRUE;
    KINFO("Input subsystem initialized.");
}

void InputShutdown() {
    initialize = FALSE;
}

void InputUpdate(f64 deltaTime) {
    if (!initialize)
        return;
    
    CopyMemory(&state.keyboardPrevious, &state.keyboardCurrent, sizeof(KeyboardState));
    CopyMemory(&state.mousePrevious, &state.mouseCurrent, sizeof(MouseState));
}

b8 InputIsKeyDown(Keys key) {
    if (!initialize)
        return FALSE;
    return state.keyboardCurrent.keys[key] == TRUE;
}

b8 InputIsKeyUp(Keys key) {
    if (!initialize)
        return FALSE;
    return state.keyboardCurrent.keys[key] == FALSE;
}

b8 InputWasKeyDown(Keys key) {
    if (!initialize)
        return FALSE;
    return state.keyboardPrevious.keys[key] == TRUE;
}

b8 InputWasKeyUp(Keys key) {
    if (!initialize)
        return FALSE;
    return state.keyboardPrevious.keys[key] == FALSE;
}

void InputProcessKey(Keys key, b8 pressed) {
    if (state.keyboardCurrent.keys[key] != pressed) {
        state.keyboardCurrent.keys[key] = pressed;
        
        EventContext context;
        context.Data.u16[0] = key;
        EventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

b8 InputIsButtonDown(Buttons button) {
    if (!initialize)
        return FALSE;
    return state.mouseCurrent.buttons[button] == TRUE;
}

b8 InputIsButtonUp(Buttons button) {
    if (!initialize)
        return FALSE;
    return state.mouseCurrent.buttons[button] == FALSE;
}

b8 InputWasButtonDown(Buttons button) {
    if (!initialize)
        return FALSE;
    return state.mousePrevious.buttons[button] == TRUE;
}

b8 InputWasButtonUp(Buttons button) {
    if (!initialize)
        return FALSE;
    return state.mousePrevious.buttons[button] == FALSE;
}

void InputGetMousePosition(i32* x, i32* y) {
    if (!initialize) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouseCurrent.x;
    *y = state.mouseCurrent.y;
}

void InputGetPreviousMousePosition(i32* x, i32* y) {
    if (!initialize) {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mousePrevious.x;
    *y = state.mousePrevious.y;
}

void InputProcessButton(Buttons button, b8 pressed) {
    if (state.mouseCurrent.buttons[button] != pressed) {
        state.mouseCurrent.buttons[button] = pressed;

        EventContext context;
        context.Data.u16[0] = button;
        EventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void InputProcessMouseMove(i16 x, i16 y) {
    if (state.mouseCurrent.x != x || state.mouseCurrent.y != y) {
        // NOTE: Enable this line if debugging.
        //KDEBUG("Mouse pos: %i, %i!", x, y);

        state.mouseCurrent.x = x;
        state.mouseCurrent.y = y;

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