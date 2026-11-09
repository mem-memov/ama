#include "semantic/initializers.h"
#include "common/path.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    AstDeclaration** earlier;
    size_t count;
    size_t capacity;
    AstTypeResolver resolve_type;
    void* context;
    Error* error;
} InitializerContext;

static AstDeclaration* find_earlier(InitializerContext* context, const char* name) {
    for (size_t index = 0; index < context->count; index++)
        if (strcmp(context->earlier[index]->name.text, name) == 0) return context->earlier[index];
    return NULL;
}

static bool append_earlier(InitializerContext* context, AstDeclaration* declaration) {
    if (context->count == context->capacity) {
        if (context->capacity > SIZE_MAX / 2 / sizeof(AstDeclaration*))
            return error_set(context->error, ERROR_LIMIT, declaration->name.location.line, "too many typed declarations");
        size_t capacity = context->capacity ? context->capacity * 2 : 16;
        AstDeclaration** earlier = realloc(context->earlier, capacity * sizeof(*earlier));
        if (!earlier) return error_set(context->error, ERROR_MEMORY, 0, "out of memory (declaration validation)");
        context->earlier = earlier;
        context->capacity = capacity;
    }
    context->earlier[context->count++] = declaration;
    return true;
}

static bool validate_initializer(InitializerContext* context, AstDeclaration* declaration, AstInitializer* initializer) {
    AstStorage expected = declaration->storage == AST_REFERENCE || declaration->storage == AST_REFERENCE_LIST
        ? AST_REFERENCE : AST_OWNED;
    initializer->source = NULL;
    if (initializer->value.kind == AST_LINE_LITERAL_VALUE) {
        return expected == AST_OWNED || error_set(context->error, ERROR_INVALID, initializer->value.location.line,
            "a literal cannot initialize a reference");
    }
    AstDeclaration* source = find_earlier(context, initializer->value.text);
    if (!source)
        return error_set(context->error, ERROR_INVALID, initializer->value.location.line,
            "initializer source '%s' must be declared earlier", initializer->value.text);
    if (strcmp(source->resolved_type, declaration->resolved_type) != 0)
        return error_set(context->error, ERROR_INVALID, initializer->value.location.line,
            "initializer type mismatch: '%s' and '%s'", source->type_path.text, declaration->type_path.text);
    if (source->storage != initializer->source_storage)
        return error_set(context->error, ERROR_INVALID, initializer->value.location.line,
            "initializer source '%s' uses the wrong storage suffix", initializer->value.text);
    AstStorage provided = initializer->source_storage;
    if (initializer->acquires_reference) {
        if (provided != AST_OWNED)
            return error_set(context->error, ERROR_INVALID, initializer->value.location.line,
                "reference acquisition requires an owned scalar");
        provided = AST_REFERENCE;
    }
    if (provided != expected)
        return error_set(context->error, ERROR_INVALID, initializer->value.location.line,
            "initializer storage does not match its destination element");
    initializer->source = source;
    return true;
}

static bool validate_declaration(InitializerContext* context, AstDeclaration* declaration) {
    if (!declaration->is_typed)
        return error_set(context->error, ERROR_INVALID, declaration->name.location.line,
            "typed declaration validation does not accept legacy defaults");
    if (find_earlier(context, declaration->name.text))
        return error_set(context->error, ERROR_INVALID, declaration->name.location.line,
            "duplicate variable base name '%s'", declaration->name.text);
    if (!normalize_path(declaration->type_path.text, declaration->resolved_type, context->error) ||
        !context->resolve_type(context->context, declaration->resolved_type, declaration->type_path.location, context->error))
        return false;
    bool list = declaration->storage == AST_OWNED_LIST || declaration->storage == AST_REFERENCE_LIST;
    if (!list && declaration->initializers && declaration->initializers->next)
        return error_set(context->error, ERROR_INVALID, declaration->name.location.line,
            "a scalar declaration permits at most one initializer");
    for (AstInitializer* initializer = declaration->initializers; initializer; initializer = initializer->next)
        if (!validate_initializer(context, declaration, initializer)) return false;
    return append_earlier(context, declaration);
}

static bool validate_block(InitializerContext* context, AstBlock* block) {
    for (AstStatement* statement = block->first; statement; statement = statement->next) {
        if (statement->kind == AST_DECLARATION) {
            if (!validate_declaration(context, statement->as.declaration)) return false;
        } else if (statement->kind == AST_BRANCH || statement->kind == AST_LOOP) {
            if (!validate_block(context, &statement->as.control.body) ||
                !validate_block(context, &statement->as.control.alternative)) return false;
        }
    }
    return true;
}

bool validate_typed_declarations(AstModule* module, AstTypeResolver resolve_type, void* context, Error* error) {
    module->declarations_validated = false;
    if (!resolve_type) return error_set(error, ERROR_INVALID, 0, "a module type resolver is required");
    InitializerContext validation = {.resolve_type = resolve_type, .context = context, .error = error};
    bool success = true;
    for (AstDeclaration* declaration = module->declarations; success && declaration; declaration = declaration->next)
        success = validate_declaration(&validation, declaration);
    if (success) success = validate_block(&validation, &module->body);
    free(validation.earlier);
    module->declarations_validated = success;
    return success;
}
