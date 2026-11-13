/**
 * @file
 * @brief Merge inferred receivers with explicit fields at their exact declaration sites.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) constructs all
 * persistent body fields in declaration order, with inferred executable sources kept out of initializer syntax.
 */
#include "../internal.h"
void inference_order_fields(SemanticType *type) {
    for (size_t width = 1; width < type->symbol_count;) {
        SemanticSymbol *cursor = type->first, *head = NULL;
        SemanticSymbol **tail = &head;
        while (cursor != NULL) {
            SemanticSymbol *left = cursor, *right = cursor;
            size_t left_count = 0, right_count = width;
            for (; left_count < width && right != NULL; left_count++) { right = right->next; }
            while (left_count != 0 || (right_count != 0 && right != NULL)) {
                SemanticSymbol *selected;
                if (left_count != 0 && (right_count == 0 || right == NULL || left->site.start <= right->site.start)) {
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
        type->first = head;
        if (width > type->symbol_count / 2) { break; }
        width *= 2;
    }
    type->last = NULL;
    type->symbol_count = 0;
    for (size_t role = 0; role < sizeof(type->counts) / sizeof(type->counts[0]); role++) { type->counts[role] = 0; }
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        symbol->ordinal = type->symbol_count++;
        symbol->role_ordinal = type->counts[symbol->role]++;
        type->last = symbol;
    }
}
