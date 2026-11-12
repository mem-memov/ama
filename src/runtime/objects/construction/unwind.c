/**
 * @file
 * @brief Report construction failure and unwind the current private ancestry.
 *
 * [LIFE-03](../../../../spec/implementation/native-interface.md#life-03) requires explicit lifecycle ordering.
 * Cleanup preserves the primary error and never destroys an uninitialized native payload.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include "internal.h"

AmaStatus ama_construction_unwind(AmaObjectOperation *operation, AmaObjectWork *work, AmaStatus primary) {
    ama_object_report(operation->world, primary);
    while (work != NULL) {
        primary = ama_object_cleanup(operation->world, work->handle, primary);
        work = work->parent;
    }
    return primary;
}
