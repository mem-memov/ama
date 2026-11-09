#include "codegen/type_plan.h"
#include "common/path.h"
#include <stdlib.h>
#include <string.h>

bool valid_type_header_name(const char* name) {
    if (!name || !*name) return false;
    for (const unsigned char* byte = (const unsigned char*)name; *byte; byte++)
        if (*byte < 32 || *byte == 127 || *byte == '"' || *byte == '\\' || *byte == '?') return false;
    return true;
}

static size_t find_type(const TypePlan* plan, const char* path) {
    for (size_t index = 0; index < plan->count; index++)
        if (strcmp(plan->types[index].source->module_path, path) == 0) return index;
    return plan->count;
}

static bool append_field(TypeFields* fields, const AstDeclaration* declaration, Error* error) {
    if (fields->count == fields->capacity) {
        if (fields->capacity > SIZE_MAX / 2 / sizeof(*fields->declarations))
            return error_set(error, ERROR_LIMIT, 0, "too many generated fields");
        size_t capacity = fields->capacity ? fields->capacity * 2 : 16;
        const AstDeclaration** declarations = realloc(fields->declarations, capacity * sizeof(*declarations));
        if (!declarations) return error_set(error, ERROR_MEMORY, 0, "out of memory (type fields)");
        fields->declarations = declarations;
        fields->capacity = capacity;
    }
    fields->declarations[fields->count++] = declaration;
    return true;
}

static bool collect_body_fields(TypeFields* fields, const AstBlock* block, Error* error) {
    for (const AstStatement* statement = block->first; statement; statement = statement->next) {
        if (statement->kind == AST_DECLARATION) {
            if (!append_field(fields, statement->as.declaration, error)) return false;
        } else if (statement->kind == AST_BRANCH || statement->kind == AST_LOOP) {
            if (!collect_body_fields(fields, &statement->as.control.body, error) ||
                !collect_body_fields(fields, &statement->as.control.alternative, error)) return false;
        }
    }
    return true;
}

static bool validate_fields(TypePlan* plan, PlannedType* type, TypeFields* fields, Error* error) {
    for (size_t index = 0; index < fields->count; index++) {
        const AstDeclaration* declaration = fields->declarations[index];
        if (!declaration->is_typed || declaration->storage != AST_OWNED)
            return error_set(error, ERROR_INVALID, declaration->name.location.line,
                "type layout generation currently requires owned scalar declarations");
        if (find_type(plan, declaration->resolved_type) == plan->count)
            return error_set(error, ERROR_INVALID, declaration->type_path.location.line,
                "missing layout provider for '%s'", declaration->resolved_type);
        if (!name_registry_register(&plan->names,
            source_identity_hash(type->source->module_path, declaration->name.text),
            type->source->module_path, declaration->name.text, error)) return false;
        const AstInitializer* initializer = declaration->initializers;
        if (!initializer) continue;
        if (initializer->next)
            return error_set(error, ERROR_INVALID, declaration->name.location.line,
                "a scalar field permits at most one initializer");
        if (initializer->value.kind == AST_LINE_LITERAL_VALUE) {
            if (!initializer->value.literal_bytes && initializer->value.literal_length)
                return error_set(error, ERROR_INVALID, initializer->value.location.line, "missing literal bytes");
        } else {
            size_t earlier = 0;
            while (earlier < index && fields->declarations[earlier] != initializer->source) earlier++;
            if (earlier == index || initializer->acquires_reference)
                return error_set(error, ERROR_INVALID, initializer->value.location.line,
                    "field initializer requires an earlier value in the same state or frame");
        }
    }
    return true;
}

static bool order_state(TypePlan* plan, size_t index, Error* error) {
    PlannedType* type = &plan->types[index];
    if (type->visit == 2) return true;
    if (type->visit == 1)
        return error_set(error, ERROR_CYCLE, 0, "recursive inline ownership involving '%s'", type->source->module_path);
    type->visit = 1;
    for (size_t field = 0; field < type->state.count; field++) {
        size_t child = find_type(plan, type->state.declarations[field]->resolved_type);
        if (!order_state(plan, child, error)) return false;
    }
    type->visit = 2;
    plan->order[plan->ordered_count++] = index;
    return true;
}

bool build_type_plan(TypePlan* plan, const TypeModule* modules, size_t count, Error* error) {
    if (!modules && count)
        return error_set(error, ERROR_INVALID, 0, "missing type modules");
    if (count > MAX_MODULES)
        return error_set(error, ERROR_LIMIT, 0, "too many type modules");
    plan->types = calloc(count ? count : 1, sizeof(*plan->types));
    plan->order = calloc(count ? count : 1, sizeof(*plan->order));
    if (!plan->types || !plan->order)
        return error_set(error, ERROR_MEMORY, 0, "out of memory (type plan)");
    plan->count = count;
    for (size_t index = 0; index < count; index++) plan->types[index].source = &modules[index];
    for (size_t index = 0; index < count; index++) {
        PlannedType* type = &plan->types[index];
        const TypeModule* source = type->source;
        char normalized[MAX_MODULE_PATH_LENGTH];
        if (!source->module_path)
            return error_set(error, ERROR_INVALID, 0, "missing type module identity");
        if (!normalize_path(source->module_path, normalized, error)) return false;
        if (strcmp(normalized, source->module_path) != 0)
            return error_set(error, ERROR_INVALID, 0, "type module paths must be normalized");
        for (size_t previous = 0; previous < index; previous++)
            if (strcmp(modules[previous].module_path, source->module_path) == 0)
                return error_set(error, ERROR_INVALID, 0, "duplicate type module '%s'", source->module_path);
        if (!name_registry_register(&plan->names, source_identity_hash(source->module_path, NULL),
            source->module_path, NULL, error)) return false;
        if (source->native_header) {
            if (!valid_type_header_name(source->native_header))
                return error_set(error, ERROR_INVALID, 0, "invalid native type header name");
            continue;
        }
        if (!name_registry_register(&plan->names, source_identity_hash(source->module_path, "$frame"),
            source->module_path, "$frame", error)) return false;
        if (!source->module || !source->module->declarations_validated)
            return error_set(error, ERROR_INVALID, 0, "type layouts require validated declarations");
        for (const AstDeclaration* declaration = source->module->declarations; declaration; declaration = declaration->next)
            if (!append_field(&type->frame, declaration, error)) return false;
        if (!collect_body_fields(&type->state, &source->module->body, error)) return false;
    }
    for (size_t index = 0; index < count; index++) {
        PlannedType* type = &plan->types[index];
        if (!validate_fields(plan, type, &type->state, error) ||
            !validate_fields(plan, type, &type->frame, error)) return false;
    }
    for (size_t index = 0; index < count; index++)
        if (!order_state(plan, index, error)) return false;
    return true;
}

void destroy_type_plan(TypePlan* plan) {
    for (size_t index = 0; index < plan->count; index++) {
        free(plan->types[index].state.declarations);
        free(plan->types[index].frame.declarations);
    }
    free(plan->types);
    free(plan->order);
    name_registry_destroy(&plan->names);
    *plan = (TypePlan){0};
}
