/** @file
 * @brief Enumerate concrete vertices by source registration and full captured identity before publication.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) requires reproducible assembly independent of discovery scheduling.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) distinguishes every captured Type.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) retains complete encoded keys, not only digest names.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Compare entire canonical key bytes, with length resolving a strict prefix.
 * A digest collision cannot silently determine ordering or Type equality.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) preserves exact full identities.
 */
static int compare(const SemanticType *left, const SemanticType *right) {
    const IdentityName *a = left->specialization->context->registry_use->candidate->name;
    const IdentityName *b = right->specialization->context->registry_use->candidate->name;
    size_t common = a->encoding_length < b->encoding_length ? a->encoding_length : b->encoding_length;
    int prefix = memcmp(a->encoding, b->encoding, common);
    if (prefix) { return prefix; }
    if (a->encoding_length == b->encoding_length) { return 0; }
    return a->encoding_length < b->encoding_length ? -1 : 1;
}

void semantic_specialization_order(SemanticProgram *program) {
    SemanticType *remaining = program->first;
    program->first = program->last = NULL;
    for (const ProjectModule *module = program->project->first; module; module = module->next) {
        SemanticType *group = NULL;
        for (SemanticType **link = &remaining; *link;) {
            SemanticType *type = *link;
            if (type->module != module) { link = &type->next; continue; }
            *link = type->next;
            SemanticType **position = &group;
            while (*position && compare(*position, type) < 0) { position = &(*position)->next; }
            type->next = *position;
            *position = type;
        }
        if (!group) { continue; }
        if (program->last) { program->last->next = group; }
        else { program->first = group; }
        program->last = group;
        while (program->last->next) { program->last = program->last->next; }
    }
    assert(!remaining);
}
