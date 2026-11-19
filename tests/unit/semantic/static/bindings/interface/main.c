/** @file
 * @brief Compose source/static inventories with exact four-sequence snapshot construction.
 */
#include "../common.c"
#include <stdint.h>

/** Interleaved interfaces include a free Type first introduced by a static
 * result definition; constructor expected foreign names remain separate.
 */
static const char *interface_source = "@=\nshared\n.other\n@<\n...explicit type\n"
    "first&\n...implicit input\n...factory\n ...foreign\nsecond[]\n...explicit type\n"
    "@>\n...static one\n...implicit result\nruntime\n...explicit type\n"
    "...static two\n...factory\n@!\n";

/** Exact fixture lookup after static acceptance, never a foreign-signature search. */
static const StaticSymbol *symbol(const StaticModule *module, const char *text) {
    const StaticSymbol *found = static_symbol_find(module, static_name_site(module->definition, text));
    assert(found != NULL);
    return found;
}

/** Compare raw byte names without assuming source or intrinsic NUL termination. */
static bool name_is(BindingName name, const char *text) {
    return name.length == strlen(text) && memcmp(name.bytes, text, name.length) == 0;
}

#include "support/source.c"
#include "support/intrinsic.c"
#include "support/failures.c"

/** Execute each independent snapshot observation and ownership partition. */
static void run(void) {
    it_freezes_four_original_interface_sequences_with_result_introduced_formals();
    it_filters_exact_fixed_identities_without_reordering_or_mutating_other_snapshots();
    it_rejects_invalid_capture_sets_before_allocating_or_changing_sources();
    it_selects_shared_constant_surfaces_and_preserves_empty_sequences();
    it_reuses_every_canonical_intrinsic_interface_without_source_fabrication();
    it_unwinds_every_source_and_intrinsic_snapshot_allocation_failure();
    it_checks_slot_count_overflow_and_first_failure_without_publication();
    it_preserves_other_sequences_when_all_static_inputs_are_fixed();
    it_rejects_duplicate_actual_runtime_names_without_publishing_partial_views();
    it_preserves_published_snapshots_when_later_construction_fails();
}

/** Return success after assertions and counted teardown of every fixture. */
int main(void) { run(); return 0; }
