/**
 * @file
 * @brief Declare project artifact construction and verified native compilation.
 *
 * [COMP-04](../../spec/implementation/compiler.md#comp-04)
 * Borrow a valid nonauthoring frontend/options through emission; publication is explicit and uses the shared conflict-safe publisher.
 */
#ifndef AMA_PROJECT_CODEGEN_H
#define AMA_PROJECT_CODEGEN_H
#include "native/scaffold/scaffold.h"

/* Strict shared frontend and all borrowed source/option strings outlive emission.
 * Artifacts use the scaffold identity registry and conflict-safe publisher. */
/**
 * Require an unused scaffold and valid nonauthoring frontend. Validate paths/model/executable
 * root, stage four complete project artifacts plus declarations, then set ready. No publication
 * occurs.
 * Component contract: [COMP-04](../../spec/implementation/compiler.md#comp-04). */
bool project_codegen_build(NativeScaffold *, ScaffoldOptions);
/**
 * Require ready unpublished source artifacts and no previously retained executable. Clear ready
 * while staging/compile/verify/link/retain proceeds; set it only after success and best-effort
 * temporary cleanup.
 * Component contract: [COMP-04](../../spec/implementation/compiler.md#comp-04). */
bool project_codegen_compile(NativeScaffold *, FILE *errors);
#endif
