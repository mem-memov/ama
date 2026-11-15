/** @file
 * @brief Build a real Ama/native/Ama chain and run explicit suspension/failure policies.
 */
#include "../nested/common.c"
#include "../nested/support/probe.c"
#include "../nested/support/link.c"
#include "child.c"
#include "parent.c"
/** The independent host can observe returned C frames and both isolated admission gates. */
static void it_supervises_nested_suspension_without_retaining_c_activations(void) {
    setup_test();
    LibraryBuild child,parent; NativeScaffold child_assembly,parent_assembly;
    build_supervision_child(&child,&child_assembly);
    build_supervision_parent(&parent,&parent_assembly);
    link_nested("generated/probe.o"); nested_execute("generated/consumer");
    native_scaffold_destroy(&parent_assembly); library_build_destroy(&parent);
    native_scaffold_destroy(&child_assembly); library_build_destroy(&child);
    destroy_test();
}
/** Register the complete independently linked composition. */
void run(void) { it_supervises_nested_suspension_without_retaining_c_activations(); }
/** Return after all C host ownership/continuation assertions have passed. */
int main(void) { run(); return EXIT_SUCCESS; }
