/**
 * @file
 * @brief Record actual module-variable occurrences independently of hoisted explicit declarations.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) excludes labels,
 * selectors and lexical locals, while an unresolved ordinary use still consumes first occurrence.
 */
#include "../internal.h"
bool inference_encounter(Inference *inference, SourceSpan name, bool *earlier) {
    for (EncounteredName *seen = inference->encountered; seen != NULL; seen = seen->next) {
        if (semantic_same_text(seen->name, name)) {
            if (earlier != NULL) { *earlier = true; }
            return true;
        }
    }
    EncounteredName *seen = semantic_allocate(inference->program, sizeof(*seen));
    if (seen == NULL) { return false; }
    *seen = (EncounteredName){inference->encountered, name};
    inference->encountered = seen;
    if (earlier != NULL) { *earlier = false; }
    return true;
}
