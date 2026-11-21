/**
 * @file
 * @brief Compare complete bounded identities and search immutable sorted prepared views.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) preserves exact spelling,
 * with no prefix fallback, Unicode normalization or preferred version.
 */
#include <string.h>
#include "internal.h"
int selection_compare(const char *a, size_t an, const char *b, size_t bn) {
    int order = memcmp(a, b, an < bn ? an : bn);
    return order != 0 ? order : an < bn ? -1 : an > bn ? 1 : 0;
}
const DependencyViewEntry *selection_find(const DependencyView *view, const char *bytes, size_t length) {
    size_t low = 0, high = view->count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        const ModulePathValue *value = view->entries[middle].identity;
        int order = selection_compare(value->text, value->length, bytes, length);
        if (order == 0) { return &view->entries[middle]; }
        if (order < 0) { low = middle + 1; }
        else { high = middle; }
    }
    return NULL;
}
