/** @file
 * @brief Resolve explicitly supplied reference facades through exact nominal checks and bounded read callbacks.
 * [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03)
 * derives generated adapter prototypes from checked access and view lifetimes;
 * [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05)
 * forbids retaining the facade or its temporary nested view after the operation.
 */
#include "codegen/internal.h"

/** Keep target scope storage private on the C stack; only a synchronous read callback sees its facade. Component contract: [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03), [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05). */
bool codegen_emit_reference_visitors(NativeScaffold *scaffold, FILE *output, bool declarations) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *reference = type->names[SCAFFOLD_REFERENCE_READ]->internal_name;
        const char *storage = type->names[SCAFFOLD_REFERENCE_STORAGE]->internal_name;
        const char *read = type->names[SCAFFOLD_READ]->internal_name;
        const char *write = type->names[SCAFFOLD_WRITE]->internal_name;
        if (fprintf(output, "static __attribute__((unused)) AmaStatus %s_Visit(const %s *reference, "
                "AmaStatus (*visit)(const %s *,void *),void *resource)%s\n",
                reference, storage, read, declarations ? ";" : " {") < 0) { return false; }
        if (declarations) { continue; }
        if (fprintf(output, "if(reference==NULL || reference->location==NULL || reference->borrow==NULL || visit==NULL) "
                "return ama_status_make(AMA_INVALID_STATE,NULL,0);\n"
                "%s_Scope target={0}; AmaStatus status=%s_Bind(reference->borrow,*reference->location,&target);\n"
                "if(status.code!=AMA_OK) return status;\n"
                "return ama_status_checked(visit(&target.read,resource));\n}\n", write, write) < 0) { return false; }
    }
    return true;
}
