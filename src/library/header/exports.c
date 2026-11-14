/** @file
 * @brief Emit factories and only selected entry/resume/read operations from typed interfaces.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) requires
 * operation selection and exact parameter/result schemas to remain explicit.
 */
#include "internal.h"
/** Emit one type-safe parameter in role declaration order, or its matching invocation name. */
static void fields(LibraryHeader *h, const LibraryExport *exported,
    SymbolRole role, bool names_only) {
    for (const SemanticSymbol *field = exported->type->first; field != NULL; field = field->next) {
        if (field->role != role) { continue; }
        const LibraryBinding *binding = library_header_binding(h->build, exported, field);
        if (names_only) {
            library_header_print(h, ", %s_%zu", role == SYMBOL_PARAMETER ? "input" : "result", field->role_ordinal);
        } else if (role == SYMBOL_PARAMETER) {
            library_header_print(h, ", const %s_Borrowed *input_%zu", binding->schema->identity->internal_name, field->role_ordinal);
        } else {
            library_header_print(h, ", %s_Value **result_%zu", binding->schema->identity->internal_name, field->role_ordinal);
        }
    }
}
/** Emit the exact selected mode's typed argument list; resume/read never accept new inputs. */
static void arguments(LibraryHeader *h, const LibraryExport *exported, unsigned mode, bool names_only) {
    if (names_only) { library_header_print(h, "context, instance"); }
    else { library_header_print(h, "%s_Context context, %s_Instance instance", h->prefix, h->prefix); }
    if (mode == LIBRARY_ENTRY) { fields(h, exported, SYMBOL_PARAMETER, names_only); }
    fields(h, exported, SYMBOL_RESULT, names_only);
    if (mode != LIBRARY_READ) { library_header_print(h, names_only ? ", outcome" : ", uint32_t *outcome"); }
}
/** Emit one declared operation and an inline ABI gate before its structure-based implementation. */
static void operation(LibraryHeader *h, const LibraryExport *exported, unsigned mode, const char *name) {
    const char *p = h->prefix, *e = exported->identity->internal_name;
    library_header_print(h,
        "/** %s on this export-bound instance; rejected admission leaves state and output owners unchanged.\n"
        " * Input pointers NULL select defaults at entry; inputs never escape the call.\n"
        " * Optional result destinations NULL skip snapshots; otherwise *destination is initialized NULL.\n"
        " * Owned result destinations must be distinct; output aliasing is an invalid request.\n"
        " * Failed snapshot construction after admission is technical context failure. No scheduler or implicit resume. */\n",
        mode == LIBRARY_ENTRY ? "Reset the interface and execute entry" : mode == LIBRARY_RESUME ? "Resume without resetting or supplying inputs" : "Read retained results without execution");
    if (mode != LIBRARY_READ) {
        library_header_print(h, "/** Accepted execution writes COMPLETED/YIELDED/FAILED; rejection preserves outcome. Outcome storage must not overlap any result owner slot. */\n");
    }
    library_header_print(h, "%s_ApiResult %s_%s_implementation(", p, e, name);
    arguments(h, exported, mode, false);
    library_header_print(h, ");\nstatic inline %s_ApiResult %s_%s(", p, e, name);
    arguments(h, exported, mode, false);
    library_header_print(h, ") {\n");
    library_header_guard(h);
    library_header_print(h, "    return %s_%s_implementation(", e, name);
    arguments(h, exported, mode, true);
    library_header_print(h, ");\n}\n");
}
void library_header_exports(LibraryHeader *h) {
    for (const LibraryExport *exported = h->build->exports; exported != NULL; exported = exported->next) {
        const char *p = h->prefix, *e = exported->identity->internal_name;
        library_header_print(h, "/** Public export %s selects nominal Ama module %s. */\n", exported->name, exported->module);
        library_header_print(h,
            "/** Construct this export's explicit instance with default fields, without executing its body.\n"
            " * *empty_instance is all-zero; accepted construction failure fails the context.\n"
            " * Later entry resets the interface even on its first invocation; body state persists. */\n"
            "%s_ApiResult %s_create_implementation(%s_Context context, %s_Instance *empty_instance);\n"
            "static inline %s_ApiResult %s_create(%s_Context context, %s_Instance *empty_instance) {\n",
            p, e, p, p, p, e, p, p);
        library_header_guard(h);
        library_header_print(h, "    return %s_create_implementation(context, empty_instance);\n}\n", e);
        if ((exported->operations & LIBRARY_ENTRY) != 0) { operation(h, exported, LIBRARY_ENTRY, "entry"); }
        if ((exported->operations & LIBRARY_RESUME) != 0) { operation(h, exported, LIBRARY_RESUME, "resume"); }
        if ((exported->operations & LIBRARY_READ) != 0) { operation(h, exported, LIBRARY_READ, "read"); }
    }
}
