/**
 * @file
 * @brief Own a retained immutable native status view with success default and value cloning.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the retained library contract and pending scalar roadmap.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include "ama_n_9c127fe74fbd6a496caca76a2828d3abd2dc744864af5c95991c28e2a2246f1e.interface.h"
#include "ama_n_9c127fe74fbd6a496caca76a2828d3abd2dc744864af5c95991c28e2a2246f1e.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_dec8e11a859272bfbb1e91a201099e67f212d80b8ab00d1371c020dd69921314 *parameters, ama_t_8837f54a827a5de0f7ed4c479590527d366f09a18a33118dc06e6a7659c93db8 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state;
    (void)parameters;
    (void)results;
    (void)services;
    *status = ama_status_success();
}

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    destination->value = ama_status_success();
    return ama_status_success();
}

/**
 * Release only successfully constructed native State. Retained scalar modules own no resource;
 * fixture-specific injected failures are returned after cleanup.
 */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}

/**
 * Copy source native data into fresh destination storage without copying runtime identity or
 * transient execution links; source stays unchanged.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}

#include "ama_n_9c127fe74fbd6a496caca76a2828d3abd2dc744864af5c95991c28e2a2246f1e.end.h"
