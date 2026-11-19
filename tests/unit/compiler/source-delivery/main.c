/** @file
 * @brief Compare exact frontend snapshots for distributed open-template sources and concrete native dependencies.
 * @see [T191 contract](../../../../docs/implementation/template-source-delivery.md)
 */
#include <unistd.h>
#include "common.c"
#include "diagnostics/render/render.h"
#include "support/inputs.c"
#include "support/refusals.c"

/** Run real source graph comparisons; compiled consumers have a separate leaf. */
static void run(void) {
    it_invalidates_exact_source_library_consumers_and_preserves_noop_snapshots();
    it_refuses_conflicting_library_names_in_both_root_orders();
    it_refuses_missing_required_library_sources_without_namespace_fallback();
    it_refuses_unclosed_delivered_templates_at_the_ordinary_selection();
}

/** Return success only after every compared frontend owner is destroyed. */
int main(void) { run(); return 0; }
