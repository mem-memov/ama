/** @file
 * @brief Share narrow closed-identity and lexical projection operations.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) preserves deterministic source registration.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) requires complete capture identity.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) permits finite nominal graph cycles.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) retains traversal bindings.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes original field binding.
 */
#ifndef AMA_SPECIALIZATION_INVENTORY_INTERNAL_H
#define AMA_SPECIALIZATION_INVENTORY_INTERNAL_H
#include "../inventory.h"
#include "semantic/internal.h"

/** Find only an exact canonical candidate, never the first source path match.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) separates different captures.
 */
SemanticType *semantic_specialization_candidate(const SemanticProgram *, const StaticRegistryCandidate *);
/** Prepare one candidate vertex before any field edge is followed.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) allows finite nominal cycles.
 */
bool semantic_specialization_vertex(SemanticProgram *, const StaticClosureEntry *);
/** Copy fixed typed fields and foreach roles into their concrete semantic owner.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) forbids reclassification.
 */
bool semantic_specialization_fields(SemanticProgram *, StaticConstraintSolver *, SemanticType *);
/** Preserve fixed traversal bindings after every source field has its semantic cell.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) retains reused and local cells.
 */
bool semantic_specialization_foreach(SemanticProgram *, SemanticType *);
/** Enumerate vertices in original Project registration order, ordering distinct
 * captures of one origin by their complete collision-checked encoded key.
 * This representation order never selects a Type or creates an instance.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) requires reproducible assembly inputs.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) preserves every capture.
 */
void semantic_specialization_order(SemanticProgram *);
#endif
