#include "codegen/internal.h"

bool codegen_emit_storage(NativeScaffold *scaffold, FILE *output) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        if (fprintf(output, "typedef struct %s_Storage {\n", name) < 0) { return false; }
        for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
            const char *field_type = member->field->symbol->list ? "AmaHandleList" : "AmaHandle";
            if (fprintf(output, "    %s %s;\n", field_type, member->identity->internal_name) < 0) { return false; }
        }
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) {
            if (fprintf(output, "    %s_State payload;\n", type->native_prefix->internal_name) < 0) { return false; }
        } else if (fputs("    unsigned char payload;\n", output) == EOF) { return false; }
        if (fprintf(output, "} %s_Storage;\nstatic AmaObjectType %s_Type;\n", name, name) < 0) { return false; }
    }
    return true;
}
