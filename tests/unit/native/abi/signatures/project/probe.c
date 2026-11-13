/**
 * @file
 * @brief Supply independently typed native signature candidates for compiler acceptance/rejection.
 *
 * Case zero is a valid control. Cases one through eight violate, respectively,
 * default, destroy, literal, clone, condition, index, entry and from_size shapes.
 * The static assertion provides one exact diagnostic marker for the type mismatch.
 */
#include "native/abi/bytes.h"
#include "native/abi/services.h"
#include "native/toolchain/gcc.h"
#ifndef AMA_ABI_CASE
#define AMA_ABI_CASE 0
#endif

/** Nominal payload for independent signature checking; no storage is constructed. */
typedef struct { size_t value; } State;
/** Nominal const input views for the entry candidate. */
typedef struct { const State *value; } Inputs;
/** Nominal writable result views for the entry candidate. */
typedef struct { State *value; } Outputs;

#if AMA_ABI_CASE == 1
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(State *);
/** Deliberately replace constructor status with an incompatible scalar result. */
int ama_abi_candidate(State *state) { (void)state; return 0; }
#elif AMA_ABI_CASE == 2
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(State *);
/** Deliberately prohibit mutation of the destructor's owned native state. */
AmaStatus ama_abi_candidate(const State *state) { (void)state; return ama_status_success(); }
#elif AMA_ABI_CASE == 3
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(State *, AmaByteView);
/** Deliberately replace the bounded byte view with a C-string pointer. */
AmaStatus ama_abi_candidate(State *state, const char *bytes) {
    (void)state; (void)bytes; return ama_status_success();
}
#elif AMA_ABI_CASE == 4
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(State *, const State *);
/** Deliberately expose mutable clone source state. */
AmaStatus ama_abi_candidate(State *state, State *source) {
    (void)state; (void)source; return ama_status_success();
}
#elif AMA_ABI_CASE == 5
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(const State *, bool *);
/** Deliberately expose mutable condition input. */
AmaStatus ama_abi_candidate(State *state, bool *result) {
    (void)state; (void)result; return ama_status_success();
}
#elif AMA_ABI_CASE == 6
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(const State *, size_t *);
/** Deliberately narrow the host-position output transport. */
AmaStatus ama_abi_candidate(const State *state, uint32_t *result) {
    (void)state; (void)result; return ama_status_success();
}
#elif AMA_ABI_CASE == 7
/** Required void entry signature, separate from all status-returning protocols. */
typedef void (*Expected)(State *, const Inputs *, Outputs *, const AmaNativeServices *, AmaStatus *);
/** Deliberately return status from an entry instead of its required void result. */
AmaStatus ama_abi_candidate(State *state, const Inputs *inputs, Outputs *outputs,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)inputs; (void)outputs; (void)services; (void)status;
    return ama_status_success();
}
#elif AMA_ABI_CASE == 8
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(State *, size_t);
/** Deliberately narrow the fixed integer bridge input. */
AmaStatus ama_abi_candidate(State *state, uint32_t value) {
    (void)state; (void)value; return ama_status_success();
}
#else
/** Required operation-specific native callable signature for this candidate. */
typedef AmaStatus (*Expected)(State *);
/** Provide a correct constructor signature so unrelated compiler failure cannot pass negatives. */
AmaStatus ama_abi_candidate(State *state) { (void)state; return ama_status_success(); }
#endif

_Static_assert(_Generic(&ama_abi_candidate, Expected: 1, default: 0),
    "AMA_EXPECTED_SIGNATURE_MISMATCH");
