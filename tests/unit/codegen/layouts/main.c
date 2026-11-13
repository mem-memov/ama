/** @file
 * @brief Compile planned storage/nominal descriptors with real over-aligned native lifecycle and borrow consumers.
 */
#include "../../compiler/common.c"
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include "codegen/codegen.h"
#include "codegen/internal.h"
#include "native/scaffold/internal.h"
#include "support/fixture.c"
#include "support/native.c"
#include "support/probe/layouts.c"
#include "support/probe/values.c"
#include "support/probe/borrows.c"
#include "support/process.c"
#include "support/cases.c"
/** Execute positive concrete ABI composition and statically distinct nominal-type cases. */
void run(void) {
    it_composes_planned_fields_with_actual_c_layout_lifecycle_and_native_borrows();
    it_rejects_equal_native_layouts_as_substitutes_for_nominal_module_identity();
}
/** Return success only after every explicit case completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
