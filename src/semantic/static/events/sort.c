/**
 * @file
 * @brief Stably order static occurrence records by original physical source position.
 *
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) makes receiver
 * classification lexical, independent of grouped AST arguments and traversal.
 */
#include "../internal.h"

/** Merge bounded sorted runs without allocation; each pass doubles their bound
 * and stops before width arithmetic can overflow. The count must match the
 * finite chain supplied by construction. Equal-site records preserve order.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
void static_sort_occurrences(StaticModule *module) {
    for (size_t width = 1; width < module->occurrence_count;) {
        StaticOccurrence *cursor = module->first_occurrence, *head = NULL;
        StaticOccurrence **tail = &head;
        while (cursor != NULL) {
            StaticOccurrence *left = cursor, *right = cursor;
            size_t left_count = 0, right_count = width;
            for (; left_count < width && right != NULL; left_count++) { right = right->next; }
            while (left_count != 0 || (right_count != 0 && right != NULL)) {
                StaticOccurrence *selected;
                if (left_count != 0 && (right_count == 0 || right == NULL || left->site.start <= right->site.start)) {
                    selected = left; left = left->next; left_count--;
                } else {
                    selected = right; right = right->next; right_count--;
                }
                *tail = selected;
                tail = &selected->next;
            }
            cursor = right;
        }
        *tail = NULL;
        module->first_occurrence = head;
        if (width > module->occurrence_count / 2) { break; }
        width *= 2;
    }
    module->last_occurrence = NULL;
    for (StaticOccurrence *record = module->first_occurrence; record != NULL; record = record->next) {
        module->last_occurrence = record;
    }
}
