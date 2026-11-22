/** @file
 * @brief Check exact membership joins, immutable source causes and failure atomicity.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../closure/common.c"
#include "../../../../../../tools/project/acquisition/closure/members/members.h"
static AcquisitionRoles roles;
static AcquisitionMembers members;
static const AcquisitionRoleRecord *requester;
/** Build a real role handle before any file cause borrows it. */
static void prepare(void) {
    begin(); acquisition_roles_init(&roles, sources.allocator);
    acquisition_members_init(&members, sources.allocator);
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/main", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &requester); assert(status == ACQUISITION_ROLE_OK);
}
/** Expire members before their roles and immutable byte owners. */
static void finish_members(void) {
    acquisition_members_destroy(&members); acquisition_roles_destroy(&roles); finish();
}
/** A public structural capture record with source-manager-owned exact bytes. */
static AcquisitionUnitEntry entry(const char *path, const char *bytes) {
    SourceSpan content = configuration_test_text(bytes);
    return (AcquisitionUnitEntry){.file = {.path = path, .content = content.source,
        .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
}
/** Every ordering/repetition of three file requests yields the same byte-sorted set. */
static void it_joins_all_bounded_membership_request_words_without_duplicates(void) {
    static const char *const paths[] = {"origin/a", "origin/b", "origin/c"};
    for (size_t word = 0; word < 243; word++) {
        prepare(); size_t code = word; unsigned expected = 0;
        for (size_t step = 0; step < 5; step++) {
            size_t index = code % 3; code /= 3; expected |= 1u << index;
            AcquisitionUnitEntry input = entry(paths[index], "same bytes");
            const AcquisitionMember *output = NULL;
            AcquisitionMemberStatus status = acquisition_members_add(&members, &input, requester, NULL,
                ACQUISITION_UNIT_MEMBER, &output); assert(status == ACQUISITION_MEMBER_OK && output);
            const AcquisitionMember *item = members.first;
            for (size_t path = 0; path < 3; path++) {
                if (!(expected & (1u << path))) { continue; }
                assert(item && !strcmp(item->path, paths[path]) && item->causes && !item->causes->next);
                item = item->next;
            }
            assert(!item);
        }
        finish_members();
    }
}
/** Shared paths retain both unit and test causes and their exact snapshot handle. */
static void it_retains_every_distinct_reason_and_bound_snapshot_contract(void) {
    prepare(); AcquisitionVersionSource version = {.version = "/a/p/v1.0.0", .directory = "origin"};
    AcquisitionUnitEntry input = entry("origin/main", "@!\n");
    const AcquisitionMember *first = NULL, *again = NULL;
    AcquisitionMemberStatus status = acquisition_members_add(&members, &input, requester, &version,
        ACQUISITION_UNIT_MEMBER, &first); assert(status == ACQUISITION_MEMBER_OK);
    for (size_t repeat = 0; repeat < 2; repeat++) {
        status = acquisition_members_add(&members, &input, requester, &version,
            ACQUISITION_TEST_MEMBER, &again); assert(status == ACQUISITION_MEMBER_OK && again == first);
    }
    assert(first->causes->source == &version && first->causes->requester == requester &&
        first->causes->kind == ACQUISITION_TEST_MEMBER && first->causes->next && !first->causes->next->next);
    finish_members();
}
/** Changed bytes, file/directory collisions and absent/invalid input cannot silently merge. */
static void it_refuses_changed_content_and_storage_kind_without_losing_previous_evidence(void) {
    prepare(); AcquisitionUnitEntry input = entry("origin/main", "");
    const AcquisitionMember *first = NULL, *output = NULL;
    AcquisitionMemberStatus status = acquisition_members_add(&members, &input, requester, NULL,
        ACQUISITION_UNIT_MEMBER, &first); assert(status == ACQUISITION_MEMBER_OK && first->content);
    AcquisitionUnitEntry changed = entry("origin/main", "different");
    status = acquisition_members_add(&members, &changed, requester, NULL, ACQUISITION_UNIT_MEMBER, &output);
    assert(status == ACQUISITION_MEMBER_CONFLICT && !output && !first->causes->next);
    changed.directory = true; changed.file.content = NULL;
    status = acquisition_members_add(&members, &changed, requester, NULL, ACQUISITION_UNIT_MEMBER, &output);
    assert(status == ACQUISITION_MEMBER_CONFLICT && !output);
    changed.file.path = "origin/empty directory";
    status = acquisition_members_add(&members, &changed, requester, NULL, ACQUISITION_UNIT_MEMBER, &output);
    assert(status == ACQUISITION_MEMBER_OK && output->directory && !output->content);
    changed.file.state = DELIVERY_FILE_ABSENT;
    status = acquisition_members_add(&members, &changed, requester, NULL, ACQUISITION_UNIT_MEMBER, &output);
    assert(status == ACQUISITION_MEMBER_INVALID && first->content && !first->causes->next);
    finish_members();
}
/** All measured insertion allocations and an old member's new cause are atomic. */
static void it_preserves_membership_and_causes_on_every_allocation_refusal(void) {
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare(); AcquisitionUnitEntry input = entry("origin/main", "bytes");
        AcquisitionMember sentinel = {0}; const AcquisitionMember *output = &sentinel;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        AcquisitionMemberStatus status = acquisition_members_add(&members, &input, requester, NULL,
            ACQUISITION_UNIT_MEMBER, &output);
        if (!failure) { assert(status == ACQUISITION_MEMBER_OK); allocations = allocation_count - before; assert(allocations); }
        else {
            assert(status == ACQUISITION_MEMBER_OUT_OF_MEMORY && !members.first && output == &sentinel); failure_at = 0;
            status = acquisition_members_add(&members, &input, requester, NULL, ACQUISITION_UNIT_MEMBER, &output);
            assert(status == ACQUISITION_MEMBER_OK);
        }
        const AcquisitionMember *first = output; output = &sentinel; failure_at = allocation_count + 1;
        status = acquisition_members_add(&members, &input, requester, NULL, ACQUISITION_TEST_MEMBER, &output);
        assert(status == ACQUISITION_MEMBER_OUT_OF_MEMORY && output == &sentinel && !first->causes->next);
        failure_at = 0; finish_members();
    }
}
/** Run finite joins, provenance, conflict and exhaustive allocation partitions. */
void run(void) {
    it_joins_all_bounded_membership_request_words_without_duplicates();
    it_retains_every_distinct_reason_and_bound_snapshot_contract();
    it_refuses_changed_content_and_storage_kind_without_losing_previous_evidence();
    it_preserves_membership_and_causes_on_every_allocation_refusal();
}
/** Exit after all membership/role/source owners retire. */
int main(void) { run(); return 0; }
