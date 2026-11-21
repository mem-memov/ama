/**
 * @file
 * @brief Verify pure normalized path identity, original component origins and bounded refusal.
 * [Normalization contract](../../../../../docs/implementation/project-configuration.md#t261-path-and-scope-contract)
 * separates stack reduction from endpoint checks and actual module loading.
 */
#include <stdio.h>
#include <stdint.h>
#include "../../common.c"
#include "source/path/normalize/internal.h"
#include "source/path/logical.h"

static SourceArena paths;

/** Own result/scratch allocations separately from borrowed original source bytes. */
static void setup_paths(void) { setup_test(); source_arena_init(&paths, sources.allocator); }
/** Retire all result/scratch blocks before original SourceText owners. */
static void finish_paths(void) { source_arena_destroy(&paths); destroy_test(); }
/** Add a bounded original operand whose source location can be independently observed. */
static SourceSpan text(const char *bytes) {
    const SourceText *source = NULL;
    assert(source_manager_add(&sources, "operand", (const unsigned char *)bytes, strlen(bytes), &source) == SOURCE_OK);
    return (SourceSpan){source, 0, strlen(bytes)};
}
/** Import known metadata, with no source provenance invented for its components. */
static const ModulePathValue *canonical(const char *bytes) {
    const ModulePathValue *value = NULL;
    SourceIssue issue = {0};
    assert(module_path_from_canonical(&paths, (const unsigned char *)bytes, strlen(bytes), &value, &issue) == MODULE_PATH_OK);
    return value;
}
/** Normalize a complete original operand with no implicit aliases or hidden context. */
static const ModulePathValue *normalized(const char *bytes, const ModulePathValue *base) {
    const ModulePathValue *value = NULL;
    SourceIssue issue = {0};
    assert(module_path_normalize(&paths, text(bytes), base, NULL, NULL, &value, &issue) == MODULE_PATH_OK);
    return value;
}

#include "support/values.c"
#include "support/model.c"
#include "support/ownership.c"

/** Run identity/provenance, algebraic model and all measured storage-failure cases. */
void run(void) {
    it_uses_explicit_directories_roots_and_initial_parent_components();
    it_defers_endpoint_validation_until_after_complete_reduction();
    it_performs_one_whole_alias_lookup_and_preserves_surviving_origins();
    it_selects_constants_after_normalization_and_rejects_incomplete_or_dotted_members();
    it_matches_all_bounded_reverse_cancellation_models();
    it_preserves_outputs_for_syntax_scope_underflow_and_storage_refusals();
    it_owns_output_bytes_independently_of_metadata_alias_and_selection_inputs();
}
/** Return only after all counted input/result owners have been released. */
int main(void) { run(); return EXIT_SUCCESS; }
