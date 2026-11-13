/**
 * @file
 * @brief Resolve declared storage forms, writability and explicit truth/index capabilities.
 *
 * [TYPE-05](../../../spec/language/declarations.md#type-05)
 * [NAME-06](../../../spec/language/declarations.md#name-06) resolves lexical
 * local cells first and forbids their use during instance construction.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) uses a declared
 * parent result only for the leading selector; indices keep the caller namespace.
 * References do not implicitly dereference or inherit conversion capability from an owned field.
 */
#include "semantic/internal.h"

/** Check declared reference/list suffix and outer writability, then initializer visibility; do not reinterpret a missing suffix as implicit dereference.
 */
static bool access_form(SemanticProgram *program, const AstAccess *syntax,
    const SemanticSymbol *symbol, bool writable, const SemanticSymbol *initializing) {
    bool selected_list = syntax->whole_list || syntax->index != NULL;
    if (syntax->reference != symbol->reference || selected_list != symbol->list) {
        return semantic_fail(program, syntax->span, "Access storage form does not match its declaration", symbol);
    }
    if (writable && symbol->role == SYMBOL_CONSTANT) {
        return semantic_fail(program, syntax->span, "Shared constant storage is immutable", symbol);
    }
    return semantic_visible(program, symbol, initializing, syntax->span);
}

bool semantic_condition(SemanticProgram *program, SemanticAccess *access,
    AbiOperation capability, SourceSpan site) {
    return semantic_conversion(program, access->symbol->type, access->reference, access->list,
        capability, site, access->symbol);
}

/** Resolve the leading symbol in its explicit domain while keeping nested index operands caller-local; the loop uses no C recursion. */
static SemanticAccess *resolve(SemanticProgram *program, SemanticType *type,
    const AstAccess *syntax, bool writable, const SemanticSymbol *initializing,
    SemanticSymbol *selected) {
    SemanticAccess *first = NULL;
    SemanticAccess **destination = &first;
    bool outer = true;
    for (const AstAccess *node = syntax; node != NULL; node = node->index) {
        SemanticSymbol *symbol = outer && selected != NULL ? selected : semantic_local_at(type, node->name, node->span);
        if (symbol != NULL && initializing != NULL) {
            semantic_fail(program, node->name, "Instance construction cannot read a runtime foreach-local binding", symbol);
            return NULL;
        }
        if (symbol == NULL) { symbol = semantic_symbol(type, node->name); }
        if (symbol == NULL) {
            semantic_fail(program, node->name, "The variable is not declared in this module", NULL);
            return NULL;
        }
        if (!access_form(program, node, symbol, outer && writable, initializing)) {
            return NULL;
        }
        SemanticAccess *access = semantic_allocate(program, sizeof(*access));
        if (access == NULL) {
            return NULL;
        }
        *access = (SemanticAccess){.syntax = node, .symbol = symbol,
            .reference = symbol->reference, .list = node->whole_list};
        if (!outer && !semantic_condition(program, access, ABI_INDEX, node->span)) {
            return NULL;
        }
        *destination = access;
        destination = &access->index;
        outer = false;
    }
    return first;
}

SemanticAccess *semantic_access(SemanticProgram *program, SemanticType *type,
    const AstAccess *syntax, bool writable, const SemanticSymbol *initializing) {
    return resolve(program, type, syntax, writable, initializing, NULL);
}

SemanticAccess *semantic_selected_access(SemanticProgram *program, SemanticType *caller,
    const AstAccess *syntax, SemanticSymbol *field) {
    return resolve(program, caller, syntax, false, NULL, field);
}
