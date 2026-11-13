/**
 * @file
 * @brief Sort inference constructs by their exact written operand locations.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) requires binding
 * events after/before selected children according to source bytes, not AST list ownership.
 */
#include "../internal.h"
void inference_sort_events(Inference *walk) {
    for (size_t width = 1; width < walk->event_count;) {
        InferenceEvent *cursor = walk->first, *head = NULL;
        InferenceEvent **tail = &head;
        while (cursor != NULL) {
            InferenceEvent *left = cursor, *right = cursor;
            size_t left_count = 0, right_count = width;
            for (; left_count < width && right != NULL; left_count++) { right = right->next; }
            while (left_count != 0 || (right_count != 0 && right != NULL)) {
                InferenceEvent *selected;
                if (left_count != 0 && (right_count == 0 || right == NULL
                        || left->site.start <= right->site.start)) {
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
        if (width > walk->event_count / 2) { break; }
        width *= 2;
    }
    walk->last = NULL;
    for (InferenceEvent *item = walk->first; item != NULL; item = item->next) { walk->last = item; }
}
