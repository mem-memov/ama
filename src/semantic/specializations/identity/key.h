/** @file
 * @brief Expose complete admitted owner keys independently of generated view/member categories.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) distinguishes complete nominal substitutions.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) retains full framed components before hashing.
 */
#ifndef AMA_SPECIALIZED_TYPE_KEY_H
#define AMA_SPECIALIZED_TYPE_KEY_H
#include "semantic/semantic.h"
#include "source/identity/identity.h"

/** Borrow one actual type in a healthy relation-checked semantic program.
 * Copy its complete structural owner descriptor into the destination arena;
 * source module paths remain borrowed. The arena must differ from the term
 * store's arena and outlive key registration. Output changes only on success;
 * partial allocations remain arena-owned and no runtime state is created.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) preserves owner identity across consumers.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed preparation ownership.
 */
IdentityStatus semantic_type_key(const SemanticProgram *, const SemanticType *, SourceArena *, QualifiedIdentity *);
#endif
