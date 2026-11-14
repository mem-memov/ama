/** @file
 * @brief Emit exact lexical intrinsic frames and their label-driven control paths.
 * [LIST-04](../../../../spec/language/lists.md#list-04) supplies typed values,
 * not synthetic namespaces; [CALL-09](../../../../spec/language/calls-and-results.md#call-09)
 * retains parents until each complete result subtree ends.
 */
#ifndef AMA_CODEGEN_LIST_CONTROL_H
#define AMA_CODEGEN_LIST_CONTROL_H
#include "codegen/internal.h"

/** Emit the exact C offset for one already classified frame domain. */
bool codegen_list_offset(FILE *, const ScaffoldType *, const StorageFrame *);
/** Borrow one typed intrinsic slot and route invalid generated extents to failure. */
bool codegen_list_frame(FILE *, const ScaffoldType *, const StorageFrame *, bool method, const char *variable);
/** Emit service opening or one contiguous append/insert prefix. */
bool codegen_list_operation(NativeScaffold *, FILE *, const ScaffoldType *, const ControlNode *, const char *);
/** Emit a method-direct result transfer with only its exact structural parent grant. */
bool codegen_list_output(FILE *, const ScaffoldType *, const ControlNode *, const char *);
/** Acquire a materialized method result without selecting an invented module field. */
bool codegen_list_value_acquire(FILE *, const ScaffoldType *, const ControlNode *, const char *);
#endif
