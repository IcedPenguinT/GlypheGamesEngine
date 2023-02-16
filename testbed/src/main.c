#include <core/Logger.h>
#include <core/Asserts.h>

int main(void) {
    KFATAL("A Fatal Test Message. %f", 3.14f);
    KERROR("A Error Test Message. %f", 3.14f);
    KWARNING("A Warning Test Message. %f", 3.14f);
    KINFO("A Info Test Message. %f", 3.14f);
    KDEBUG("A Debug Test Message. %f", 3.14f);
    KTRACE("A Trace Test Message. %f", 3.14f);
    KASSERT(1 == 0);

    return 0;
}