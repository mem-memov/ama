/** @file
 * @brief Compose an actual isolated child archive with a native parent and independent C host.
 */
#include "common.c"
#include "child.c"
#include "support/probe.c"
#include "support/link.c"
#include "parent.c"
#include "copy.c"
/** Real compiled C calls exercise the native resource exception without exposing private child storage. */
static void it_embeds_independent_child_resources_through_checked_public_apis(void) {
    setup_test();
    LibraryBuild child,parent; NativeScaffold child_assembly,parent_assembly;
    build_child(&child,&child_assembly);
    build_parent(&parent,&parent_assembly);
    link_nested(NULL); nested_execute("generated/consumer");
    native_scaffold_destroy(&parent_assembly); library_build_destroy(&parent);
    native_scaffold_destroy(&child_assembly); library_build_destroy(&child);
    destroy_test();
}
/** Register actual nested compiled consumption before the static noncopyability checks. */
void run(void) {
    it_embeds_independent_child_resources_through_checked_public_apis();
    it_rejects_ordinary_and_continuation_copy_of_the_child_owner();
    it_rejects_reference_transport_across_the_component_boundary();
}
/** Return only after the separate public consumer process completes. */
int main(void) { run(); return EXIT_SUCCESS; }
