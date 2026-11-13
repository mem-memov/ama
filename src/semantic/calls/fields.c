/**
 * @file
 * @brief Resolve named/positional interface fields and reject duplicate bindings.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Direction role and exact storage form must match the callee declaration; placeholders still consume positions.
 * [LIST-04](../../../spec/language/lists.md#list-04) supplies intrinsic field
 * descriptors without fabricating declaration spans.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) keeps the
 * interface declaration and earlier binding together when rejecting a duplicate.
 */
#include "internal.h"
#include "semantic/lists/internal.h"

/** Resolve only source-declared fields; intrinsic interfaces use immutable descriptors instead. */
static bool ordinary(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding) {
    SemanticType *type = statement->namespace_type;
    SymbolRole role = statement->syntax->as.call.mode == AST_CONSTANT_READ ? SYMBOL_CONSTANT
        : argument->kind == AST_INPUT ? SYMBOL_PARAMETER : SYMBOL_RESULT;
    SemanticSymbol *field = argument->label == NULL ? semantic_interface_position(type, role, argument->position)
        : semantic_interface_name(type, role, argument->label->name);
    if (field == NULL) {
        SemanticSymbol *related = argument->label == NULL ? NULL : semantic_symbol(type, argument->label->name);
        return semantic_fail(program, argument->span, "No interface field matches this binding", related);
    }
    if (argument->label != NULL && (argument->label->reference != field->reference
            || argument->label->whole_list != field->list)) {
        return semantic_fail(program, argument->label->span, "Binding label storage form differs from its field", field);
    }
    binding->field = field;
    binding->type = field->type;
    binding->position = field->role_ordinal;
    binding->reference = field->reference;
    binding->list = field->list;
    return true;
}
bool semantic_call_field(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding) {
    bool valid = statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION
        ? semantic_list_binding(program, statement, argument, binding)
        : ordinary(program, statement, argument, binding);
    if (!valid) { return false; }
    for (SemanticBinding *earlier = statement->bindings; earlier != NULL; earlier = earlier->next) {
        if (earlier->position == binding->position && earlier->syntax->kind == argument->kind) {
            semantic_fail(program, argument->span, "The interface field is bound more than once", binding->field);
            size_t index = program->error.related_count;
            const char *name = binding->field == NULL ? NULL : statement->namespace_type->module->source->logical_path;
            program->related[index] = (DiagnosticRelated){{.span = earlier->syntax->span}, "earlier binding", name};
            program->error.related_count = index + 1;
            program->error.related = program->related;
            return false;
        }
    }
    return true;
}
