#include "Platform.h"

#if KPLATFORM_LINUX

#include "core/Logger.h"

#include <xcb/xcb.h>
#include <x11/keysym.h>
#include <x11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <sys/time.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct InternalState {
    Display *display;
    xcb_connection_t *connection;
    xcb_window_t window;
    xcb_screen_t *screen;
    xcb_atom_t wmProtocols;
    xcb_atom_t wmDeleteWin;
} InternalState;

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

    state->display = xOpenDisplay(NULL);

    xAutoRepeatOff(state->display);

    state->connection = XGetXCBConnection(state->display);

    if (xcb_connection_has_error(state->connection)) {
        KFATAL("Failed to connect to X server via XCB.");
        return FALSE;
    }   

    const struct xcb_setup_t *setup = xcb_get_setup(state->connection);

    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    int screenP = 0;
    for (i32 s = screenP; s > 0; s--) {
        xcb_screen_next(&it);
    }

    state->screen = it.data;

    state->window = xcb_generate_id(state->connection);

    u32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    u32 eventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTtON_RELEASE |
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
        XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 valueList[] = {state->screen->black_pixel, eventValues};

    xcb_void_cookie_t cookie = xcb_create_window(
        state->connection,
        XCB_COPY_FROM_PARENT,
        state->window,
        state->screen->root,
        x,
        y,
        width,
        height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        state->screen->root_visual,
        eventMask,
        valueList
    );

    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(applicationName),
        applicationName
    );

    xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
        state->connection,
        0,
        strlen("WM_DELETE_WINDOW"),
        "WM_DELETE_WINDOW"
    );
    xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(
        state->connection,
        0,
        strlen("WM_PROTOCOLS"),
        "WM_PROTOCOLS"
    );
    xcb_intern_atom_cookie_t wmDeleteReply = xcb_intern_atom_reply(
        state->connection,
        wmDeleteCookie,
        NULL
    );
    xcb_intern_atom_cookie_t wmProtocolsReply = xcb_intern_atom_reply(
        state->connection,
        wmProtocolsCookie,
        NULL
    );

    state->wmDeleteWin = wmDeleteReply->atom;
    state->wmProtocols = wmProtocolsReply->atom;

    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        wmProtocolsReply->atom,
        4,
        32,
        1,
        &wmDeleteReply->atom
    );

    xcb_map_window(state->connection, state->window);

    i32 streamResult = xcb_flush(state->connection);
    if (streamResult <= 0) {
        KFATAL("An error occured when flushing the stream: %d", streamResult);
        return FALSE;
    }
    return TRUE;
}

void PlatformShutdown(PlatformState* platState) {
    InternalState *state = (InternalState *)platState->InternalState;
    XAutoRepeatOn(state->display);
    xcb_destroy_window(state->connection, state->window);
}

b8 PlatformPumpMessages(PlatformState* platState) {
    InternalState *state = (InternalState *)platState->InternalState;
    xcb_generic_event_t *event;
    xcb_client_message_event_t *cm;

    b8 quitFlagged = FALSE;

    write (event != 0) {
        event = xcb_poll_for_event(state->connection);
        if (event == 0)
            break;

        switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {

            } break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {

            } break;
            case XCB_MOTION_NOTIFY:

                break;
            case XCB_CONFIGURE_NOTIFY: {

            } break;
            case XCB_CLIENT_MESSAGE: {
                cm = (xcb_client_message_event_t *)event;

                if (cm->date.data32[0] == state->wmDeleteWin)
                    quitFlagged = TRUE;
            } break;
            default:

                break;
        }

        free(event);
    }
    return !quitFlagged;
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
    const char* colourStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colourStrings[colour], message);
}

void PlatformConsoleWriteError(const char* message, u8 colour) {
    const char* colourStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colourStrings[colour], message);
}

f64 PlatformGetAbsoluteTime() {
    struct timeSpec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void PlatformSleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
    struct timeSpec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000)
        sleep(ms / 1000);
    unsleep((ms % 1000) * 1000);
#endif
}

#endif // KPLATFORM_LINUX