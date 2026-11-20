/** @file
 * @brief Compose source/static inventories with exact four-sequence snapshot construction.
 */
#include "../common.c"
#include <stdint.h>

/** Ordinary Type formals are introduced by runtime pairs, including a result;
 * named static outputs belong only to the separate graph fixture.
 */
static const char *interface_source = "@=\nshared\n/test/fixture/v1.0.0/src/other\n@<\nfirst&\n.explicit type\n"
    "second[]\n.implicit input\nfactory\n.factory\n@>\nruntime\n.explicit type\n"
    "introduced\n.implicit result\n@!\n";
static const char *graph_source = "@...\n.static one\n.implicit result\n"
    ".static two\n.explicit type\n";

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
    it_freezes_independent_ordinary_and_graph_interface_sequences();
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
