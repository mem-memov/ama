/** @file
 * @brief Join captured file memberships without losing causes or accepting changed bytes.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains exact snapshots;
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) merges overlapping units
 * into a least set while preserving [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03)
 * source provenance.
 */
#include "members.h"
#include <string.h>
/** Equality compares bytes, not source pointers, because overlapping unit captures
 * may own independent immutable observations under
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07). */
static bool same_content(const SourceText *left, const SourceText *right) {
    return source_length(left) == source_length(right) &&
        !memcmp(source_bytes(left), source_bytes(right), source_length(left));
}
AcquisitionMemberStatus acquisition_members_add(AcquisitionMembers *members, const AcquisitionUnitEntry *input,
    const AcquisitionRoleRecord *requester, const AcquisitionVersionSource *source, AcquisitionMemberKind kind,
    const AcquisitionMember **output) {
    if (!members || !input || !requester || !output || !input->file.path || !*input->file.path ||
        input->file.state != DELIVERY_FILE_PRESENT ||
        (input->directory ? input->file.content != NULL : input->file.content == NULL) ||
        kind < ACQUISITION_UNIT_MEMBER || kind > ACQUISITION_DECLARED_INPUT) {
        return ACQUISITION_MEMBER_INVALID;
    }
    AcquisitionMember **position = &members->first;
    while (*position && strcmp((*position)->path, input->file.path) < 0) { position = &(*position)->next; }
    AcquisitionMember *member = *position;
    bool exists = member && !strcmp(member->path, input->file.path);
    if (exists && (member->directory != input->directory ||
        (!member->directory && !same_content(member->content, input->file.content)))) { return ACQUISITION_MEMBER_CONFLICT; }
    AcquisitionMemberCause *cause = exists ? member->causes : NULL;
    while (cause && (cause->requester != requester || cause->source != source || cause->kind != kind)) { cause = cause->next; }
    bool retained_cause = cause != NULL;
    if (!cause) {
        cause = source_arena_allocate(&members->arena, sizeof(*cause));
        if (!cause) { return ACQUISITION_MEMBER_OUT_OF_MEMORY; }
        *cause = (AcquisitionMemberCause){.requester = requester, .source = source, .kind = kind};
    }
    if (!exists) {
        member = source_arena_allocate(&members->arena, sizeof(*member));
        const char *path = member ? source_arena_copy(&members->arena, input->file.path) : NULL;
        if (!path) { return ACQUISITION_MEMBER_OUT_OF_MEMORY; }
        *member = (AcquisitionMember){.next = *position, .path = path,
            .content = input->file.content, .directory = input->directory};
    }
    /* The remaining join is non-fallible: a failed preparation cannot install
     * even a new cause on an old input, nor a new member without its cause. */
    if (!retained_cause) { cause->next = member->causes; member->causes = cause; }
    if (!exists) { *position = member; }
    *output = member; return ACQUISITION_MEMBER_OK;
}
