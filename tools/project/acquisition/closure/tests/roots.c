/** @file
 * @brief Collect original runtime definitions as candidates for exact test admission.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) requires a concrete
 * directly associated program; [TPL-06](../../../../../spec/language/templates.md#tpl-06)
 * retains open source definitions without inventing concrete instances.
 */
#include "roots.h"
#include <string.h>
AcquisitionTestRootsStatus acquisition_test_roots(SourceArena *arena, const DeliverySourceDefinition *definitions,
    const char *directory, const AcquisitionTestRoot **output) {
    if (!arena || !directory || !*directory || !output) {
        return ACQUISITION_TEST_ROOTS_INVALID;
    }
    size_t length = strlen(directory);
    AcquisitionTestRoot *first = NULL;
    for (const DeliverySourceDefinition *definition = definitions; definition; definition = definition->next) {
        const char *identity = definition->identity;
        if ((definition->kind != DELIVERY_SOURCE_CONCRETE && definition->kind != DELIVERY_SOURCE_PARAMETERIZED) ||
            strncmp(identity, directory, length) || identity[length] != '/' ||
            !identity[length + 1] || strchr(identity + length + 1, '/')) { continue; }
        AcquisitionTestRoot *root = source_arena_allocate(arena, sizeof(*root));
        if (!root) { return ACQUISITION_TEST_ROOTS_OUT_OF_MEMORY; }
        *root = (AcquisitionTestRoot){.definition = definition};
        AcquisitionTestRoot **position = &first;
        while (*position && strcmp((*position)->definition->identity, identity) < 0) {
            position = &(*position)->next;
        }
        root->next = *position; *position = root;
    }
    if (!first) { return ACQUISITION_TEST_ROOTS_MISSING; }
    *output = first;
    return ACQUISITION_TEST_ROOTS_OK;
}
