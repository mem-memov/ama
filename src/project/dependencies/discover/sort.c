/**
 * @file
 * @brief Sort collected request sites by exact original-byte position before registry admission.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) requires
 * reproducible discovery. Stable merge passes preserve every edge, independent
 * of AST storage layout, selected-action links and loop-header folding.
 */
#include "internal.h"
void project_sort_paths(DependencyPaths *paths) {
    for (size_t width = 1; width < paths->count;) {
        DependencyPath *cursor = paths->first, *head = NULL;
        DependencyPath **tail = &head;
        while (cursor != NULL) {
            DependencyPath *left = cursor, *right = cursor;
            size_t left_count = 0, right_count = width;
            for (; left_count < width && right != NULL; left_count++) { right = right->next; }
            while (left_count != 0 || (right_count != 0 && right != NULL)) {
                DependencyPath *selected;
                if (left_count != 0 && (right_count == 0 || right == NULL
                        || left->path.start <= right->path.start)) {
                    selected = left;
                    left = left->next;
                    left_count--;
                } else {
                    selected = right;
                    right = right->next;
                    right_count--;
                }
                *tail = selected;
                tail = &selected->next;
            }
            cursor = right;
        }
        *tail = NULL;
        paths->first = head;
        if (width > paths->count / 2) { break; }
        width *= 2;
    }
}
