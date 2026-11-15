/** @file
 * @brief Compile public C consumers, reject incompatible interfaces and execute scalar ABI guards.
 */
#include "common.c"
#include "support/consumers.c"
#include "support/rejections.c"
#include "support/gate.c"
#include "support/literals.c"
#include "support/collisions.c"
#include "support/symbols.c"
/** Register each independently meaningful consumer/header contract. */
void run(void) {
    it_enumerates_only_declared_public_component_definitions();
    it_rejects_cross_component_hash_collisions_in_headers_and_bootstrap();
    it_rejects_unsupported_targets_before_header_publication();
    it_preserves_metadata_octets_without_injecting_c_source();
    it_compiles_independent_consumers_and_two_component_headers();
    it_rejects_incompatible_headers_and_private_type_dependencies();
    it_checks_abi_before_calling_structure_based_implementations();
}
/** Return after real C compilation and the generated guard execution have passed. */
int main(void) { run(); return EXIT_SUCCESS; }
