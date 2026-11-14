/** @file
 * @brief Hold one independent native resource and an address-sensitive retained counter.
 * The address is a native relocation invariant, not an Ama handle or a borrowed
 * reference to another instance. Clone construction always establishes a new one.
 */
#ifndef AMA_MINIMAL_COUNTER_STATE_H
#define AMA_MINIMAL_COUNTER_STATE_H
#include <stddef.h>
#include <stdint.h>
/** Own one counter value and one 16-byte allocation at a stable native address. */
struct AMA_NATIVE(State) {
    size_t calls;
    uintptr_t address;
    unsigned char *resource;
};
#endif
