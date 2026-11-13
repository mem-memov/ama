/**
 * @file
 * @brief Visit declarations and operations in exact physical source order after structural collection.
 * [NAME-04](../../../spec/language/declarations.md#name-04) reports the later
 * duplicate against the earlier declaration regardless of AST storage layout.
 * Stable iterative merges also retain a loop owner before its same-site command.
 */
#include "internal.h"
void semantic_walk_sort(SemanticWalk *walk) {
    for (size_t width = 1; width < walk->count;) {
        SemanticWalkItem *cursor = walk->first, *head = NULL;
        SemanticWalkItem **tail = &head;
        while (cursor != NULL) {
            SemanticWalkItem *left = cursor, *right = cursor;
            size_t left_count = 0, right_count = width;
            for (; left_count < width && right != NULL; left_count++) { right = right->next; }
            while (left_count != 0 || (right_count != 0 && right != NULL)) {
                SemanticWalkItem *selected;
                if (left_count != 0 && (right_count == 0 || right == NULL
                        || left->node->span.start <= right->node->span.start)) {
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
        walk->first = head;
        if (width > walk->count / 2) { break; }
        width *= 2;
    }
    walk->last = NULL;
    for (SemanticWalkItem *item = walk->first; item != NULL; item = item->next) { walk->last = item; }
}
