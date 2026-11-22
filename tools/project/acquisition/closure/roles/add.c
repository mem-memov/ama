/** @file
 * @brief Join delivery roles atomically while retaining distinct original causes.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) permits only context
 * promotion and one active expansion. Prepare allocations before publishing any
 * role, reason or queue mutation, preserving [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 * Original requesting edges remain part of reproducible delivery evidence.
 * [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03)
 */
#include "roles.h"
#include <string.h>
/** Compare exact causal edges, not filenames or diagnostic text; provenance stays
 * attached to the original requester under
 * [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03). */
static bool same_reason(const AcquisitionReason *reason, AcquisitionReasonKind kind,
    const AcquisitionRoleRecord *requester, SourceSpan origin) {
    return reason->kind == kind && reason->requester == requester &&
        reason->origin.source == origin.source && reason->origin.start == origin.start &&
        reason->origin.end == origin.end;
}
AcquisitionRoleStatus acquisition_roles_add(AcquisitionRoles *roles, const char *identity,
    AcquisitionRole role, AcquisitionReasonKind kind, const AcquisitionRoleRecord *requester,
    SourceSpan origin, const AcquisitionRoleRecord **output) {
    if (!identity || !*identity || !output ||
        (role != ACQUISITION_CONTEXT && role != ACQUISITION_ACTIVE) ||
        kind < ACQUISITION_SELECTED || kind > ACQUISITION_TEST ||
        (origin.source ? !source_span_valid(origin) : origin.start || origin.end)) {
        return ACQUISITION_ROLE_INVALID;
    }
    AcquisitionRoleRecord *record = NULL;
    bool known_requester = !requester;
    for (AcquisitionRoleRecord *item = roles->records; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { record = item; }
        if (item == requester) { known_requester = true; }
    }
    if (!known_requester) { return ACQUISITION_ROLE_INVALID; }
    AcquisitionReason *reason = record ? record->reasons : NULL;
    while (reason && !same_reason(reason, kind, requester, origin)) { reason = reason->next; }
    if (!reason) {
        reason = source_arena_allocate(&roles->arena, sizeof(*reason));
        if (!reason) { return ACQUISITION_ROLE_OUT_OF_MEMORY; }
        *reason = (AcquisitionReason){.requester = requester, .kind = kind, .origin = origin};
    }
    bool fresh = !record;
    if (fresh) {
        record = source_arena_allocate(&roles->arena, sizeof(*record));
        const char *copy = record ? source_arena_copy(&roles->arena, identity) : NULL;
        if (!copy) { return ACQUISITION_ROLE_OUT_OF_MEMORY; }
        *record = (AcquisitionRoleRecord){.identity = copy, .role = ACQUISITION_CONTEXT};
    }
    /* All fallible work precedes publication. Role joins never downgrade an
     * active record, and only the context-to-active edge appends queue work. */
    if (fresh) { record->next = roles->records; roles->records = record; }
    bool retained = false;
    for (AcquisitionReason *item = record->reasons; item; item = item->next) {
        if (item == reason) { retained = true; break; }
    }
    if (!retained) { reason->next = record->reasons; record->reasons = reason; }
    if (role == ACQUISITION_ACTIVE && record->role == ACQUISITION_CONTEXT) {
        record->role = ACQUISITION_ACTIVE;
        if (roles->last_pending) { roles->last_pending->pending_next = record; }
        else { roles->pending = record; }
        roles->last_pending = record;
    }
    *output = record;
    return ACQUISITION_ROLE_OK;
}
