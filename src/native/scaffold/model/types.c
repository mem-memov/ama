/**
 * @file
 * @brief Register exact nominal view/member identities and stable native artifact names.
 *
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01)
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) requires
 * all native prefix identities registered before the sorted provider plan is built.
 * Sort the retained supported logical identities explicitly; forward/reference cycles do not recursively expand C layouts.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) Member identities use exact semantic names for both declaration forms.
 */
#include <string.h>
#include "native/scaffold/internal.h"
#include "identity.h"

/** Create ordered field/member records with original names and category-separated identities; all strings/records belong to the scaffold.
 * Component contract: [NAME-01](../../../../spec/implementation/compiler.md#name-01), [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [TYPE-06](../../../../spec/language/declarations.md#type-06). */
static bool members(NativeScaffold *scaffold, ScaffoldType *type) {
    ScaffoldMember **next = &type->members;
    for (const StorageField *field = type->plan->first; field != NULL; field = field->next) {
        SourceSpan span = field->symbol->name;
        char *text = scaffold_allocate(scaffold, span.end - span.start + 1);
        ScaffoldMember *member = scaffold_allocate(scaffold, sizeof(*member));
        if (text == NULL || member == NULL) {
            return false;
        }
        memcpy(text, source_bytes(span.source) + span.start, span.end - span.start);
        member->field = field;
        member->identity = scaffold_identity(scaffold, type->plan->type, field->symbol->role == SYMBOL_STATE
            ? IDENTITY_STATE_MEMBER : IDENTITY_INTERFACE_MEMBER, text, span);
        if (member->identity == NULL) {
            return false;
        }
        *next = member;
        next = &member->next;
    }
    return true;
}

/** Register every declared view role and native prefix before member emission; no locale, transliteration or sequential name counter.
 * Component contract: [NAME-01](../../../../spec/implementation/compiler.md#name-01), [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [TYPE-06](../../../../spec/language/declarations.md#type-06). */
static bool names(NativeScaffold *scaffold, ScaffoldType *type) {
    static const char *const roles[] = {"read", "write", "reference-storage", "reference-read", "reference-write",
        "list-storage", "list-read", "list-write", "reference-list-storage", "reference-list-read",
        "reference-list-write", "parameters", "results"};
    /* Only concrete @C leaves establish AMA_NATIVE contexts. Ordinary model
     * prefixes identify full Types, not native companion namespaces.
     * [NAME-05](../../../../spec/implementation/native-interface.md#name-05)
     * [TPL-13](../../../../spec/language/templates.md#tpl-13) */
    IdentityCategory prefix = type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
        ? IDENTITY_NATIVE_NAMESPACE : IDENTITY_TYPE;
    type->native_prefix = scaffold_identity(scaffold, type->plan->type, prefix, NULL, (SourceSpan){0});
    for (size_t index = 0; index < SCAFFOLD_ROLE_COUNT; index++) {
        type->names[index] = scaffold_identity(scaffold, type->plan->type, IDENTITY_TYPE, roles[index], (SourceSpan){0});
        if (type->names[index] == NULL) {
            return false;
        }
    }
    return type->native_prefix != NULL && members(scaffold, type);
}

/** Derive selected native companion and generated artifact names from explicit physical/module-qualified identities; preserve first path failure.
 * Component contract: [NAME-01](../../../../spec/implementation/compiler.md#name-01), [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [TYPE-06](../../../../spec/language/declarations.md#type-06). */
static bool paths(NativeScaffold *scaffold, ScaffoldType *type) {
    type->ama_path = scaffold_absolute(scaffold, source_path(type->plan->type->module->source->text));
    if (type->ama_path == NULL) {
        return false;
    }
    type->header_path = scaffold_join(scaffold, type->ama_path, "", ".h");
    type->body_path = scaffold_join(scaffold, type->ama_path, "", ".c");
    const char *prefix = type->native_prefix->internal_name;
    type->interface_name = scaffold_join(scaffold, prefix, "", ".interface.h");
    type->begin_name = scaffold_join(scaffold, prefix, "", ".begin.h");
    type->end_name = scaffold_join(scaffold, prefix, "", ".end.h");
    type->template_name = scaffold_join(scaffold, prefix, "", ".template.c");
    type->test_name = scaffold_join(scaffold, prefix, "", ".test-template.c");
    return scaffold->error.message == NULL;
}

/** Preserve logical path order and break captured-owner ties by complete framed
 * encodings, rather than discovery order or digest bytes.
 * [COMP-05](../../../../spec/implementation/compiler.md#comp-05) requires deterministic output.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) retains exact identity equality.
 */
static bool precedes(const ScaffoldType *left, const ScaffoldType *right) {
    int path = strcmp(left->plan->type->module->source->logical_path, right->plan->type->module->source->logical_path);
    if (path) { return path < 0; }
    size_t a = left->native_prefix->encoding_length, b = right->native_prefix->encoding_length;
    int key = memcmp(left->native_prefix->encoding, right->native_prefix->encoding, a < b ? a : b);
    return key < 0 || (key == 0 && a < b);
}

bool scaffold_model(NativeScaffold *scaffold) {
    for (const PlannedType *plan = scaffold->frontend->plan.first; plan != NULL; plan = plan->next) {
        ScaffoldType *type = scaffold_allocate(scaffold, sizeof(*type));
        if (type == NULL) {
            return false;
        }
        type->plan = plan;
        if (!scaffold_hooks(scaffold, type) || !names(scaffold, type) || !paths(scaffold, type)) {
            return false;
        }
        ScaffoldType **position = &scaffold->types;
        while (*position != NULL && precedes(*position, type)) {
            position = &(*position)->next;
        }
        type->next = *position;
        *position = type;
        if (plan == scaffold->frontend->plan.entry) {
            scaffold->selected = type;
        }
    }
    return scaffold->selected != NULL && scaffold_providers(scaffold);
}

const ScaffoldType *native_scaffold_type(const NativeScaffold *scaffold, const SemanticType *semantic) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type == semantic) {
            return type;
        }
    }
    return NULL;
}
