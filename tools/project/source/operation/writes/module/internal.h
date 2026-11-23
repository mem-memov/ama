/** @file
 * @brief Separate copied module-companion authority from parsed alias normalization.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires authoritative original source/companion ownership before editing.
 */
#ifndef AMA_SOURCE_OPERATION_MODULE_WRITE_INTERNAL_H
#define AMA_SOURCE_OPERATION_MODULE_WRITE_INTERNAL_H
#include "module.h"
/** Bind actual C/P companion and original package scope; retain typed failed path
 * evidence and arena-owned path scratch without parsing a draft. */
SourceOperationModuleWriteStatus source_operation_module_write_admit(SourceOperationModuleWrite *,
    const SourceOperationModuleWriteInputs *);
#endif
