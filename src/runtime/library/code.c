/** @file
 * @brief Maintain independent code pins and exact snapshot-owner membership.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) permits
 * unload only after all code-dependent releases have become unnecessary.
 */
#include <assert.h>
#include <threads.h>
#include "code.h"
/** Acquire only the finite metadata section; ownership excludes concurrent release of one owner. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
static void code_lock(AmaLibraryCodePins *code) {
    while (atomic_exchange_explicit(&code->locked, true, memory_order_acquire)) { thrd_yield(); }
}
/** Publish the changed finite registry before allowing another metadata operation. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
static void code_unlock(AmaLibraryCodePins *code) {
    atomic_store_explicit(&code->locked, false, memory_order_release);
}
bool ama_library_code_retain(AmaLibraryCodePins *code) {
    code_lock(code);
    bool available = code->pins != SIZE_MAX;
    if (available) { code->pins++; }
    code_unlock(code);
    return available;
}
bool ama_library_code_release(AmaLibraryCodePins *code) {
    code_lock(code);
    bool available = code->pins != 0;
    if (available) { code->pins--; }
    code_unlock(code);
    return available;
}
bool ama_library_code_register(AmaLibraryCodePins *code, AmaLibraryCodeOwner *owner, const void *schema) {
    code_lock(code);
    bool available = !owner->registered && code->pins != SIZE_MAX;
    if (available) {
        *owner = (AmaLibraryCodeOwner){.next=code->first,.schema=schema,.registered=true};
        if (code->first != NULL) { code->first->previous=owner; }
        code->first=owner;
        code->pins++;
    }
    code_unlock(code);
    return available;
}
void ama_library_code_unregister(AmaLibraryCodePins *code, AmaLibraryCodeOwner *owner) {
    code_lock(code);
    if (owner->registered) {
        assert(code->pins != 0);
        if (owner->previous == NULL) { code->first=owner->next; }
        else { owner->previous->next=owner->next; }
        if (owner->next != NULL) { owner->next->previous=owner->previous; }
        code->pins--;
        *owner=(AmaLibraryCodeOwner){0};
    }
    code_unlock(code);
}
bool ama_library_code_contains(AmaLibraryCodePins *code, const void *address, const void *schema) {
    code_lock(code);
    AmaLibraryCodeOwner *owner=code->first;
    while (owner != NULL && (const void *)owner != address) { owner=owner->next; }
    bool present=owner != NULL && owner->schema == schema;
    code_unlock(code);
    return present;
}
bool ama_library_code_quiescent(AmaLibraryCodePins *code) {
    code_lock(code);
    bool empty=code->pins == 0;
    code_unlock(code);
    return empty;
}
