/** @file
 * @brief Exercise code dependencies without creating any Ama context or pool state.
 */
#include "runtime/library/code.h"
#include "runtime/library/native.h"
/** Exact registry membership rejects foreign storage and schema before dereference. */
static void it_tracks_code_pins_and_snapshot_owners_without_counter_wrap(void) {
    AmaLibraryCodePins code={0}, foreign={0};
    AmaLibraryCodeOwner first={0}, middle={0}, last={0};
    static const char schema, other_schema;
    assert(ama_library_code_quiescent(&code));
    assert(!ama_library_code_release(&code));
    assert(ama_library_code_register(&code,&first,&schema));
    assert(ama_library_code_register(&code,&middle,&schema));
    assert(ama_library_code_register(&code,&last,&schema));
    assert(!ama_library_code_register(&code,&first,&schema));
    assert(!ama_library_code_contains(&foreign,&first,&schema));
    assert(!ama_library_code_contains(&code,&first,&other_schema));
    assert(!ama_library_code_contains(&code,(const void *)(uintptr_t)1,&schema));
    assert(ama_library_code_contains(&code,&middle,&schema));
    ama_library_code_unregister(&code,&middle);
    assert(last.next==&first && first.previous==&last);
    ama_library_code_unregister(&code,&first);
    assert(last.next==NULL);
    assert(ama_library_code_retain(&code));
    ama_library_code_unregister(&code,&last);
    ama_library_code_unregister(&code,&last);
    assert(!ama_library_code_quiescent(&code));
    assert(ama_library_code_release(&code));
    assert(ama_library_code_quiescent(&code));
    code.pins=SIZE_MAX;
    assert(!ama_library_code_retain(&code));
    assert(!ama_library_code_register(&code,&first,&schema));
    assert(code.pins==SIZE_MAX && !first.registered);
    code.pins=0;
}

/** Native credits outlive contexts without owning any Ama storage; output validation and exhaustion preserve ownership. */
static void it_retains_native_code_through_independent_move_only_leases(void) {
    AmaLibraryCodePins code = {0};
    AmaNativeCodeProvider provider = ama_library_code_provider(&code);
    AmaNativeCodeLease first = {0}, second = {0};
    AmaStatus status = ama_native_code_acquire(&provider, &first);
    assert(status.code == AMA_OK && first.resource == &code && code.pins == 1);
    status = ama_native_code_acquire(&provider, &first);
    assert(status.code == AMA_INVALID_STATE && code.pins == 1);
    status = ama_native_code_acquire(&provider, &second);
    assert(status.code == AMA_OK && code.pins == 2);
    ama_native_code_drop(&first);
    assert(first.resource == NULL && first.release == NULL && code.pins == 1);
    ama_native_code_drop(&first);
    assert(code.pins == 1);
    ama_native_code_drop(&second);
    assert(ama_library_code_quiescent(&code));
    code.pins = SIZE_MAX;
    status = ama_native_code_acquire(&provider, &first);
    assert(status.code == AMA_LIMIT_EXCEEDED && first.resource == NULL && first.release == NULL && code.pins == SIZE_MAX);
    code.pins = 0;
    AmaNativeCodeProvider resident = {0};
    status = ama_native_code_acquire(&resident, &first);
    assert(status.code == AMA_OK && first.resource == NULL && first.release == NULL && code.pins == 0);
    AmaNativeCodeProvider malformed = ama_library_code_provider(NULL);
    status = ama_native_code_acquire(&malformed, &first);
    assert(status.code == AMA_INVALID_STATE && first.resource == NULL);
    malformed = (AmaNativeCodeProvider){.resource = &code};
    status = ama_native_code_acquire(&malformed, &first);
    assert(status.code == AMA_INVALID_STATE && first.resource == NULL);
    status = ama_native_code_acquire(NULL, &first);
    assert(status.code == AMA_INVALID_STATE && first.resource == NULL);
    status = ama_native_code_acquire(&provider, NULL);
    assert(status.code == AMA_INVALID_STATE && ama_library_code_quiescent(&code));
    ama_native_code_drop(NULL);
}
