/** @file
 * @brief Compose shared-member identities from complete semantic owner substitutions.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) separates shared parts of distinct specializations.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) requires complete independently framed key components.
 */
#ifndef AMA_SPECIALIZED_MEMBER_KEY_H
#define AMA_SPECIALIZED_MEMBER_KEY_H
#include "semantic/semantic.h"
#include "source/identity/identity.h"

/** Build a constant-category qualified key for one actual shared declaration.
 * Borrow the valid semantic program and its original source/closure owners.
 * The destination arena owns copied member text, component array and any static
 * descriptor; it must outlive key encoding and differ from the term-store arena.
 * The result is metadata only, not a name registration or shared runtime value.
 * Output changes only on success; partial allocation remains arena-owned.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) preserves full owner identity.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) forbids delimiter parsing and digest equality.
 */
IdentityStatus semantic_constant_key(const SemanticProgram *, const SemanticSymbol *, SourceArena *, QualifiedIdentity *);
#endif
