/**
 * @file
 * @brief Bound checked native views to one already-running receiver and synchronous operation.
 *
 * [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03) requires
 * nominal typed views of already bound storage. [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05)
 * keeps this linear runtime authority outside authored payloads and forbids escape.
 */
#ifndef AMA_PROGRAM_NATIVE_BORROW_H
#define AMA_PROGRAM_NATIVE_BORROW_H
#include "runtime/program/program.h"

/** Runtime-only linear borrow owner; initialize to zero and never copy while active.
 * All generated views expire on end, before Ama dispatch or cleanup continues.
 * It owns no payload, handle lifetime or allocation. Native authors never receive this record.
 * Component contract: [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03), [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05). */
typedef struct {
    AmaProgram *program;
    AmaHandle receiver;
    bool active;
} AmaNativeBorrowScope;

/** Distinguish ordinary available reads from the exact running receiver's existing invocation authority. Component contract: [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03), [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05). */
typedef enum {
    AMA_NATIVE_BORROW_READ,
    AMA_NATIVE_BORROW_RECEIVER
} AmaNativeBorrowRole;

/** Begin for the exact current Running receiver with a valid attached return link.
 * Reject world reentry/failure and invalid receiver before acquiring the operation
 * gate. Output scope changes only on success; program/world outlive the scope.
 * Component contract: [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03), [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05). */
AmaStatus ama_native_borrow_begin(AmaProgram *, AmaHandle, AmaNativeBorrowScope *);
/** End once before leaving native code, releasing only this owner's world gate and clearing authority.
 * No allocation, payload use or callback is performed; views must not escape.
 * Component contract: [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03), [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05). */
AmaStatus ama_native_borrow_end(AmaNativeBorrowScope *);
/** Resolve a nominally typed published Live cell under an active scope.
 * Read checks availability. Receiver permits only this scope's exact running
 * identity, not arbitrary Busy targets. Output changes only on success; never
 * infer the pool layout from the caller's intended C cast or expose private metadata to authors.
 * Component contract: [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03), [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05). */
AmaStatus ama_native_borrow_cell(const AmaNativeBorrowScope *, AmaHandle, uint64_t,
    AmaNativeBorrowRole, AmaPoolCell **);
#endif
