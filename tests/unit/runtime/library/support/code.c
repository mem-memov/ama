/** @file
 * @brief Exercise code dependencies without creating any Ama context or pool state.
 */
#include "runtime/library/code.h"
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
