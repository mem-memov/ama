/** @file
 * @brief Separate closed write-source admission, preparation and extra metadata assembly.
 * [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one finite preflight set before any physical publication.
 */
#ifndef AMA_SOURCE_OPERATION_WRITE_JOIN_INTERNAL_H
#define AMA_SOURCE_OPERATION_WRITE_JOIN_INTERNAL_H
#include "join.h"
#include "../../../copy/image/internal.h"
/** Validate genuine common capture/copy/delete owners and finite supplied text. */
SourceOperationWritesStatus source_operation_writes_admit(const SourceOperationWritesInputs *);
/** Append one unique typed copied-text result, borrowing immutable edits. */
SourceOperationWritesStatus source_operation_writes_text(SourceOperationWrites *, const SourceOperationPreparedWrite *);
/** Append a unique extra row; normalize new identity classes without host probes. */
SourceOperationWritesStatus source_operation_writes_extra(SourceOperationWrites *, SourceImageChange,
    SourceOperationExtraOwner);
/** Admit actual new-version structure and required empty package rows. */
SourceOperationWritesStatus source_operation_writes_initializations(SourceOperationWrites *);
/** Join typed package-configuration results over the exact source facts. */
SourceOperationWritesStatus source_operation_writes_configurations(SourceOperationWrites *);
/** Join each accepted module-alias writer only over this exact copy/C/P and its
 * actual regular companion mapping. Borrow immutable producer edits and use the
 * common prepared-text duplicate/ownership gate; never interpret other opaque
 * companions. Failure leaves the joined prefix owned and the outer join invalid. */
SourceOperationWritesStatus source_operation_writes_modules(SourceOperationWrites *);
/** Require actual P destination membership for an expected regular configuration result. */
const SourceFileMapping *source_operation_writes_file(const SourceFileMap *, const char *);
#endif
