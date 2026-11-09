#ifndef AMA_CODEGEN_TYPE_PLAN_H
#define AMA_CODEGEN_TYPE_PLAN_H

#include "codegen/types.h"
#include "common/names.h"

typedef struct {
    const AstDeclaration** declarations;
    size_t count;
    size_t capacity;
} TypeFields;

typedef struct {
    const TypeModule* source;
    TypeFields state;
    TypeFields frame;
    unsigned int visit;
} PlannedType;

typedef struct {
    PlannedType* types;
    size_t count;
    size_t* order;
    size_t ordered_count;
    NameRegistry names;
} TypePlan;

bool build_type_plan(TypePlan* plan, const TypeModule* modules, size_t count, Error* error);
void destroy_type_plan(TypePlan* plan);
bool valid_type_header_name(const char* name);

#endif
