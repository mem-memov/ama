/** @file
 * @brief Verify coherent source identities through real parsed scopes and prepared package selection.
 * @see [T234 contract](../../../../../../docs/implementation/acquisition-plan.md)
 */
#include <errno.h>
#include <sys/stat.h>
#include "../common.c"
#include "../../../../../../tools/project/acquisition/sources/sources.h"
static AcquisitionSources plan;
static AcquisitionSourcesIssue problem;
static const char *const commit_a = "1111111111111111111111111111111111111111";
static const char *const commit_b = "2222222222222222222222222222222222222222";
/** Parse an actual repository scope with explicit raw transport-key fields. */
static void scope(ConfigurationDocument *owner, const char *url, const char *directory, const char *revision) {
    char text[1024];
    int written = snprintf(text, sizeof(text), "[repository /a/p]\nurl = %s\ndirectory = %s\n%s%s%s",
        url, directory, revision ? "revision = " : "", revision ? revision : "", revision ? "\n" : "");
    assert(written > 0 && (size_t)written < sizeof(text));
    ConfigurationStatus status = document(owner, text, CONFIGURATION_PACKAGE); assert(status == CONFIGURATION_OK);
}
/** Plan one actual normalized identity against its requesting immutable scope. */
static AcquisitionSourceRequest request(ConfigurationDocument *owner, const char *name) {
    AcquisitionSourceRequest value = {0};
    AcquisitionSourcesStatus status = acquisition_sources_request(&plan, owner, identity(name), &value, &problem);
    assert(status == ACQUISITION_SOURCES_OK); return value;
}
/** Publish one explicit transport observation; this helper never invokes Git. */
static void resolve(const AcquisitionSourceRequest *value, const char *commit) {
    AcquisitionSourcesStatus status = acquisition_snapshot_resolve(&plan, value->snapshot, commit, &problem);
    assert(status == ACQUISITION_SOURCES_OK);
}
#include "support/snapshots.c"
#include "support/conflicts.c"
#include "support/prepared.c"
#include "support/ownership.c"
#include "support/selection.c"

/** Run identity, conflict, local-gate and measured failure partitions explicitly. */
void run(void) {
    it_coalesces_exact_snapshot_requests_and_waits_for_explicit_resolution();
    it_distinguishes_default_revision_and_aliases_equal_resolved_commits();
    it_refuses_all_unequal_source_contract_fields_without_replacing_the_prior_version();
    it_rejects_foreign_commit_handles_and_preserves_single_assignment();
    it_refuses_unmapped_and_foreign_requests_without_growing_indexes();
    it_uses_actual_prepared_whole_versions_before_routing_or_source_indexing();
    it_recovers_every_request_resolution_and_binding_allocation_refusal();
    it_routes_every_actual_alias_mask_match_once_in_either_inventory_order();
}
/** Exit only after every operation/catalog/source owner has retired cleanly. */
int main(void) { run(); return 0; }
