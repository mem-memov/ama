/** @file
 * @brief Check concrete ordinary operations and finite candidate groups before constant execution planning.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) requires full ordinary validation.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) distinguishes registered identities from checked groups.
 */
#ifndef AMA_SPECIALIZATION_CHECK_H
#define AMA_SPECIALIZATION_CHECK_H
#include "inventory.h"

/** Consume closed required Types in a fresh semantic program and validate native
 * protocols, all defaults/bodies, clone and eager/constant dependency relations.
 * Publish candidate-local certificates only after every ordinary check passes;
 * then require the finite registry dependency group to close. Failure retains
 * partial metadata and source-backed diagnostics for destruction. This operation
 * creates no constant execution plan or lowered/runtime instance.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) does not exempt unreachable operations.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) requires every outgoing dependency.
 */
SemanticStatus semantic_specializations_check(SemanticProgram *, StaticClosurePlan *, const SemanticNativeAuthoring *);
#endif
