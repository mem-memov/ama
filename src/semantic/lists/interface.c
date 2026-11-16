/** @file
 * @brief Share exact immutable method/field descriptors between receiver inference and full list binding validation.
 * [LIST-04](../../../spec/language/lists.md#list-04) defines one fixed interface
 * per entry; the table never models a runtime action selector or hidden instance.
 */
#include <string.h>
#include "interface.h"
/** One finite input/result table borrowing static descriptor arrays; bounds determine arity exactly. Component contract: [LIST-04](../../../spec/language/lists.md#list-04). */
typedef struct {
    const char *name;
    const SemanticListField *inputs;
    size_t input_count;
    const SemanticListField *results;
    size_t result_count;
} MethodInterface;
static const SemanticListField source[] = {{"source", true}};
static const SemanticListField insert[] = {{"position", false}, {"source", true}};
static const SemanticListField position[] = {{"position", false}};
static const SemanticListField swap[] = {{"first_position", false}, {"second_position", false}};
static const SemanticListField element[] = {{"element", true}};
static const SemanticListField length[] = {{"value", false}};
static const MethodInterface interfaces[SEMANTIC_LIST_METHOD_COUNT] = {
    {"append", source, 1, element, 1}, {"insert", insert, 2, element, 1},
    {"remove", position, 1, NULL, 0}, {"clear", NULL, 0, NULL, 0},
    {"swap", swap, 2, NULL, 0}, {"length", NULL, 0, length, 1}
};
/** Compare the complete byte span with one fixed ASCII interface identifier. Component contract: [LIST-04](../../../spec/language/lists.md#list-04). */
static bool matches(SourceSpan name, const char *text) {
    size_t size = strlen(text);
    return name.end - name.start == size && memcmp(source_bytes(name.source) + name.start, text, size) == 0;
}
bool semantic_list_method(SourceSpan name, SemanticListMethod *method) {
    for (size_t index = 0; index < SEMANTIC_LIST_METHOD_COUNT; index++) {
        if (matches(name, interfaces[index].name)) { *method = (SemanticListMethod)index; return true; }
    }
    return false;
}
/** The same finite table defines parsing and display names; no instance or allocation is introduced. Component contract: [LIST-04](../../../spec/language/lists.md#list-04). */
const char *semantic_list_method_name(SemanticListMethod method) {
    return (size_t)method < SEMANTIC_LIST_METHOD_COUNT ? interfaces[method].name : NULL;
}
const SemanticListField *semantic_list_position(SemanticListMethod method, bool result, size_t ordinal) {
    if ((size_t)method >= SEMANTIC_LIST_METHOD_COUNT) { return NULL; }
    const MethodInterface *interface = &interfaces[method];
    size_t count = result ? interface->result_count : interface->input_count;
    const SemanticListField *fields = result ? interface->results : interface->inputs;
    return ordinal < count ? &fields[ordinal] : NULL;
}
const SemanticListField *semantic_list_name(SemanticListMethod method, bool result, SourceSpan name, size_t *ordinal) {
    for (size_t index = 0;; index++) {
        const SemanticListField *field = semantic_list_position(method, result, index);
        if (field == NULL) { return NULL; }
        if (matches(name, field->name)) { *ordinal = index; return field; }
    }
}
