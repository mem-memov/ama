#ifndef AMA_PROJECT_CODEGEN_H
#define AMA_PROJECT_CODEGEN_H
#include "native/scaffold/scaffold.h"

/* Strict shared frontend and all borrowed source/option strings outlive emission.
 * Artifacts use the scaffold identity registry and conflict-safe publisher. */
bool project_codegen_build(NativeScaffold *, ScaffoldOptions);
bool project_codegen_compile(NativeScaffold *, FILE *errors);
#endif
