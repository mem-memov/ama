/** @file
 * @brief Append an original constant member to its complete canonical owner key without parsing display text.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) assigns each specialization its own shared declarations.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) retains all framed identity components.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial key preparation.
 */
#include <stdint.h>
#include <string.h>
#include "key.h"
#include "semantic/specializations/identity/key.h"

/** Match pointer membership before following caller-supplied declaration payloads.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) scopes a member to one concrete owner.
 */
static const SemanticType *owner(const SemanticProgram *program, const SemanticSymbol *symbol) {
    for (const SemanticType *type = program->first; type; type = type->next) {
        for (const SemanticSymbol *field = type->first; field; field = field->next) {
            if (field == symbol) { return type; }
        }
    }
    return NULL;
}

IdentityStatus semantic_constant_key(const SemanticProgram *program, const SemanticSymbol *symbol,
    SourceArena *arena, QualifiedIdentity *output) {
    if (!program || program->status != SEMANTIC_OK || !program->relations_valid || !arena || !output) {
        return IDENTITY_INVALID;
    }
    const SemanticType *type = owner(program, symbol);
    if (!type || symbol->role != SYMBOL_CONSTANT || !source_span_valid(symbol->name)) { return IDENTITY_INVALID; }
    QualifiedIdentity base;
    IdentityStatus status = semantic_type_key(program, type, arena, &base);
    if (status != IDENTITY_OK) { return status; }
    if (base.member_count == SIZE_MAX || base.member_count + 1 > SIZE_MAX / sizeof(const char *)) { return IDENTITY_LIMIT; }
    size_t length = symbol->name.end - symbol->name.start;
    if (length == SIZE_MAX) { return IDENTITY_LIMIT; }
    char *member = source_arena_allocate(arena, length + 1);
    if (!member) { return IDENTITY_OUT_OF_MEMORY; }
    memcpy(member, source_bytes(symbol->name.source) + symbol->name.start, length);
    member[length] = 0;
    const char **parts = source_arena_allocate(arena, (base.member_count + 1) * sizeof(*parts));
    if (!parts) { return IDENTITY_OUT_OF_MEMORY; }
    for (size_t index = 0; index < base.member_count; index++) { parts[index] = base.members[index]; }
    parts[base.member_count] = member;
    base.category = IDENTITY_CONSTANT;
    base.members = parts;
    base.member_count++;
    base.declaration = symbol->name;
    *output = base;
    return IDENTITY_OK;
}
