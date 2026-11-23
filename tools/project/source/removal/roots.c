/** @file
 * @brief Preserve explicit client choices instead of silently redirecting dangling entry/export roots.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks original roots independently of replacement availability and authored edges.
 */
#include <string.h>
#include "roots.h"
#include "source/path/logical.h"
/** Validate exact original ownership before dereferencing a supplied definition. */
static bool valid(const DeliverySourceDefinition *definitions, const SourceClientRoot *root) {
    const DeliverySourceDefinition *definition = definitions;
    while (definition && definition != root->definition) { definition = definition->next; }
    if (!definition || !definition->identity || root->kind < SOURCE_CLIENT_ENTRY || root->kind > SOURCE_CLIENT_EXPORT
        || root->action < SOURCE_CLIENT_KEEP || root->action > SOURCE_CLIENT_MAP) { return false; }
    if (root->action != SOURCE_CLIENT_MAP) { return root->destination == NULL; }
    return root->destination && module_path_canonical((const unsigned char *)root->destination, strlen(root->destination));
}
/** Refuse malformed client cycles before either common cut or final-setting
 * consumers traverse them; ordinary finite requests need no guessed size cap. */
static bool finite(const SourceClientRoot *roots) {
    const SourceClientRoot *slow = roots, *fast = roots;
    while (fast && fast->next) {
        slow = slow->next; fast = fast->next->next;
        if (slow == fast) { return false; }
    }
    return true;
}
SourceRemovalStatus source_removal_client_roots(SourceArena *arena, const DeliverySourceDefinition *definitions,
    const SourceClientRoot *roots, const char *const *removed, size_t count, const SourceClientRootBlocker **output) {
    if (!arena || !output || (count && !removed) || !finite(roots)) { return SOURCE_REMOVAL_INVALID; }
    for (size_t index = 0; index < count; index++) {
        if (!removed[index] || !*removed[index]) { return SOURCE_REMOVAL_INVALID; }
    }
    SourceClientRootBlocker *first = NULL, **tail = &first;
    for (const SourceClientRoot *root = roots; root; root = root->next) {
        if (!valid(definitions, root)) { return SOURCE_REMOVAL_INVALID; }
        if (root->action != SOURCE_CLIENT_KEEP) { continue; }
        bool deleted = false;
        for (size_t index = 0; index < count; index++) {
            if (!strcmp(root->definition->identity, removed[index])) { deleted = true; break; }
        }
        if (!deleted) { continue; }
        SourceClientRootBlocker *blocker = source_arena_allocate(arena, sizeof(*blocker));
        if (!blocker) { return SOURCE_REMOVAL_OUT_OF_MEMORY; }
        *blocker = (SourceClientRootBlocker){.root = root};
        *tail = blocker; tail = &blocker->next;
    }
    *output = first;
    return first ? SOURCE_REMOVAL_BLOCKED : SOURCE_REMOVAL_OK;
}
