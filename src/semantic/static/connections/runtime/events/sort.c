/** @file
 * @brief Stably order actual runtime operands by original source position without allocation.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes source-order receiver classification.
 */
#include "../../internal.h"

/** Bottom-up finite merges double width only within count bounds. Each pass
 * preserves identities/cardinality and chooses the left item on equal sites.
 * Time is O(E log E), auxiliary storage O(1); no source/Type value changes.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) retains actual lexical occurrence order.
 */
void static_runtime_sort(StaticRuntimeModule *module) {
    for (size_t width = 1; width < module->event_count;) {
        StaticRuntimeEvent *cursor = module->first_event, *head = NULL;
        StaticRuntimeEvent **tail = &head;
        while (cursor) {
            StaticRuntimeEvent *left = cursor, *right = cursor;
            size_t left_count = 0, right_count = width;
            for (; left_count < width && right; left_count++) { right = right->next; }
            while (left_count || (right_count && right)) {
                StaticRuntimeEvent *selected;
                if (left_count && (!right_count || right == NULL || left->site.start <= right->site.start)) {
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
        module->first_event = head;
        if (width > module->event_count / 2) { break; }
        width *= 2;
    }
    module->last_event = NULL;
    for (StaticRuntimeEvent *event = module->first_event; event; event = event->next) { module->last_event = event; }
}
