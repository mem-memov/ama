/** @file
 * @brief Assemble explicit inspection helpers once in a generated context's runtime image.
 * [DEBUG-01](../../../spec/implementation/diagnostics.md#debug-01) shares compiled immutable metadata;
 * [PAR-02](../../../spec/language/async-and-parallelism.md#par-02) keeps execution authority leased.
 */
#ifndef AMA_RUNTIME_DEBUGGER_ASSEMBLY_H
#define AMA_RUNTIME_DEBUGGER_ASSEMBLY_H
#include "session/admission.c"
#include "session/context.c"
#include "metadata/schema.c"
#include "instances/resolve.c"
#include "instances/snapshot.c"
#include "instances/enumerate.c"
#include "instances/fields.c"
#include "instances/cursors.c"
#include "instances/native.c"
#endif
