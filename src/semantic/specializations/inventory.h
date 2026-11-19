/** @file
 * @brief Project required closed Type descriptors into the ordinary checker inventory.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) shares only full-key type metadata.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) preserves lexical fields.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) requires ordinary operation checks.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) separates inventory from admission.
 */
#ifndef AMA_SPECIALIZATION_INVENTORY_H
#define AMA_SPECIALIZATION_INVENTORY_H
#include "semantic/semantic.h"
#include "semantic/static/closure/plan.h"

/** Consume a healthy finally closed Type worklist in a fresh semantic program
 * borrowing its original project after source/native definition admission. Owners of closure and sources outlive program.
 * All vertices precede field linking, so finite nominal cycles do not unfold.
 * Success publishes names_valid and the borrowed Type-closure certificate; native, operation and lifecycle checks
 * remain required. Failed partial metadata remains owned and destroyable.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) requires subsequent ordinary checks.
 */
SemanticStatus semantic_specialization_names(SemanticProgram *, StaticClosurePlan *);
#endif
