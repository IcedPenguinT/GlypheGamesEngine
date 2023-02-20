#include <core/Logger.h>
#include <core/Asserts.h>

#include <platform/Platform.h>

int main(void) {
    KFATAL("A Fatal Test Message. %f", 3.14f);
    KERROR("A Error Test Message. %f", 3.14f);
    KWARNING("A Warning Test Message. %f", 3.14f);
    KINFO("A Info Test Message. %f", 3.14f);
    KDEBUG("A Debug Test Message. %f", 3.14f);
    KTRACE("A Trace Test Message. %f", 3.14f);

    PlatformState state;
    if (PlatformStartup(&state, "GlypheGames Engine Testbed", 100, 100, 1280, 720)){
        while (TRUE) {
            PlatformPumpMessages(&state);
        }
    }
    PlatformShutdown(&state);

    return 0;
}