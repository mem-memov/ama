/** @file
 * @brief Check delivery role joins against finite state models and failed allocations.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../closure/common.c"
#include "../../../../../../tools/project/acquisition/closure/roles/roles.h"
/** Exhaust all five-event words over context/active requests for three identities.
 * The independent oracle is a pair of bitsets: present and ever active. Queue
 * claims after each event must equal newly active bits, without duplicates. */
static void it_matches_all_bounded_role_request_sequences(void) {
    static const char *const names[] = {"/a/p/v1.0.0/src/a", "/a/p/v1.0.0/src/b", "/a/p/v1.0.0/src/c"};
    for (size_t sequence = 0; sequence < 7776; sequence++) {
        begin(); AcquisitionRoles roles;
        acquisition_roles_init(&roles, sources.allocator);
        unsigned present = 0, active = 0; size_t code = sequence;
        const AcquisitionRoleRecord *handles[3] = {0};
        for (size_t step = 0; step < 5; step++) {
            size_t symbol = code % 6; code /= 6;
            size_t index = symbol / 2; bool activate = symbol % 2;
            unsigned bit = 1u << index; bool newly_active = activate && !(active & bit);
            present |= bit; if (activate) { active |= bit; }
            const AcquisitionRoleRecord *record = NULL;
            AcquisitionRoleStatus status = acquisition_roles_add(&roles, names[index],
                activate ? ACQUISITION_ACTIVE : ACQUISITION_CONTEXT,
                ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &record);
            assert(status == ACQUISITION_ROLE_OK && record);
            if (handles[index]) { assert(record == handles[index]); }
            handles[index] = record;
            assert(record->role == (active & bit ? ACQUISITION_ACTIVE : ACQUISITION_CONTEXT));
            const AcquisitionRoleRecord *claimed = acquisition_roles_take(&roles);
            assert(claimed == (newly_active ? record : NULL));
            assert(!acquisition_roles_take(&roles));
            unsigned observed = 0;
            for (const AcquisitionRoleRecord *item = roles.records; item; item = item->next) {
                for (size_t name = 0; name < 3; name++) {
                    if (!strcmp(item->identity, names[name])) { observed |= 1u << name; }
                }
                assert(item->reasons && !item->reasons->next);
            }
            assert(observed == present);
        }
        acquisition_roles_destroy(&roles); finish();
    }
}
/** Discovery of a back edge preserves evidence without restarting either node. */
static void it_expands_cyclic_dependencies_once_and_preserves_distinct_reasons(void) {
    begin(); AcquisitionRoles roles; acquisition_roles_init(&roles, sources.allocator);
    const AcquisitionRoleRecord *a = NULL, *b = NULL, *again = NULL;
    char name[] = "/a/p/v1.0.0/src/a";
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, name, ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &a); assert(status == ACQUISITION_ROLE_OK);
    name[1] = 'z'; assert(!strcmp(a->identity, "/a/p/v1.0.0/src/a"));
    const AcquisitionRoleRecord *claimed = acquisition_roles_take(&roles); assert(claimed == a);
    SourceSpan origin = configuration_test_text("actual original dependency");
    status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/b", ACQUISITION_ACTIVE,
        ACQUISITION_DEPENDENCY, a, origin, &b); assert(status == ACQUISITION_ROLE_OK);
    claimed = acquisition_roles_take(&roles); assert(claimed == b);
    for (size_t repeat = 0; repeat < 2; repeat++) {
        status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/a", ACQUISITION_ACTIVE,
            ACQUISITION_DEPENDENCY, b, origin, &again);
        assert(status == ACQUISITION_ROLE_OK && again == a && a->expanded);
    }
    assert(a->reasons->requester == b && a->reasons->origin.source == origin.source);
    assert(a->reasons->next && !a->reasons->next->next && !acquisition_roles_take(&roles));
    acquisition_roles_destroy(&roles); finish();
}
/** Promotion appends behind already queued work and records survive queue exhaustion. */
static void it_keeps_fifo_order_across_promotion_and_later_discovery(void) {
    begin(); AcquisitionRoles roles; acquisition_roles_init(&roles, sources.allocator);
    const AcquisitionRoleRecord *a = NULL, *b = NULL, *c = NULL, *output = NULL;
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/a", ACQUISITION_CONTEXT,
        ACQUISITION_TOPOLOGY, NULL, (SourceSpan){0}, &a); assert(status == ACQUISITION_ROLE_OK);
    status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/b", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &b); assert(status == ACQUISITION_ROLE_OK);
    status = acquisition_roles_add(&roles, a->identity, ACQUISITION_ACTIVE,
        ACQUISITION_DEPENDENCY, b, (SourceSpan){0}, &output); assert(status == ACQUISITION_ROLE_OK && output == a);
    const AcquisitionRoleRecord *claimed = acquisition_roles_take(&roles); assert(claimed == b);
    claimed = acquisition_roles_take(&roles); assert(claimed == a && !acquisition_roles_take(&roles));
    status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/c", ACQUISITION_ACTIVE,
        ACQUISITION_TEST, a, (SourceSpan){0}, &c); assert(status == ACQUISITION_ROLE_OK);
    claimed = acquisition_roles_take(&roles); assert(claimed == c && !acquisition_roles_take(&roles));
    acquisition_roles_destroy(&roles); finish();
}
/** Every insertion allocation can fail without publishing a record or pending work. */
static void it_preserves_visible_membership_on_every_insertion_allocation_failure(void) {
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        begin(); AcquisitionRoles roles; acquisition_roles_init(&roles, sources.allocator);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        AcquisitionRoleRecord sentinel = {0}; const AcquisitionRoleRecord *record = &sentinel;
        AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/m", ACQUISITION_ACTIVE,
            ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &record);
        if (!failure) { allocations = allocation_count - before; assert(status == ACQUISITION_ROLE_OK && allocations); }
        else {
            assert(status == ACQUISITION_ROLE_OUT_OF_MEMORY && record == &sentinel && !roles.records && !roles.pending);
            failure_at = 0;
            status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/m", ACQUISITION_ACTIVE,
                ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &record);
            assert(status == ACQUISITION_ROLE_OK && record != &sentinel);
        }
        acquisition_roles_destroy(&roles); finish();
    }
}
/** A failed new cause cannot promote an existing context or discard its old cause. */
static void it_refuses_foreign_causes_and_failed_promotions_without_queue_changes(void) {
    begin(); AcquisitionRoles roles; acquisition_roles_init(&roles, sources.allocator);
    const AcquisitionRoleRecord *record = NULL, *output = NULL;
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/a/p/v1.0.0/src/m", ACQUISITION_CONTEXT,
        ACQUISITION_TOPOLOGY, NULL, (SourceSpan){0}, &record); assert(status == ACQUISITION_ROLE_OK);
    failure_at = allocation_count + 1;
    status = acquisition_roles_add(&roles, record->identity, ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &output);
    assert(status == ACQUISITION_ROLE_OUT_OF_MEMORY && !output && record->role == ACQUISITION_CONTEXT);
    assert(!record->reasons->next && !acquisition_roles_take(&roles)); failure_at = 0;
    AcquisitionRoleRecord foreign = {0};
    status = acquisition_roles_add(&roles, record->identity, ACQUISITION_ACTIVE,
        ACQUISITION_DEPENDENCY, &foreign, (SourceSpan){0}, &output);
    assert(status == ACQUISITION_ROLE_INVALID && !output && record->role == ACQUISITION_CONTEXT);
    status = acquisition_roles_add(&roles, "", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &output); assert(status == ACQUISITION_ROLE_INVALID);
    status = acquisition_roles_add(&roles, record->identity, ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &output); assert(status == ACQUISITION_ROLE_OK && output == record);
    const AcquisitionRoleRecord *claimed = acquisition_roles_take(&roles); assert(claimed == record);
    acquisition_roles_destroy(&roles); finish();
}
/** Invoke finite joins, real cyclic discovery and ownership/refusal partitions. */
void run(void) {
    it_matches_all_bounded_role_request_sequences();
    it_expands_cyclic_dependencies_once_and_preserves_distinct_reasons();
    it_keeps_fifo_order_across_promotion_and_later_discovery();
    it_preserves_visible_membership_on_every_insertion_allocation_failure();
    it_refuses_foreign_causes_and_failed_promotions_without_queue_changes();
}
/** Retire all operation/source owners before normal success. */
int main(void) { run(); return 0; }
