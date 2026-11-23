/** @file
 * @brief Prove exact physical relation equality in both directions, including opaque byte ownership.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids both dropped material and additions without an original selected cause.
 */
#include "internal.h"
#include <string.h>
/** Original key equality is insufficient without exact final key, kind and bytes. */
static bool equal(const SourceFileMapping *left, const SourceFileMapping *right) {
    if (strcmp(left->source, right->source) || strcmp(left->destination, right->destination)
        || left->original->directory != right->original->directory) { return false; }
    return left->original->directory ||
        (source_length(left->original->file.content) == source_length(right->original->file.content)
            && !memcmp(source_bytes(left->original->file.content), source_bytes(right->original->file.content),
                source_length(left->original->file.content)));
}
/** Unique source keys make exact row membership a complete set inclusion test. */
static bool subset(const SourceFileMap *left, const SourceFileMap *right, const char **failed) {
    for (const SourceFileMapping *row = left->first; row; row = row->next) {
        const SourceFileMapping *match = right->first;
        while (match && strcmp(match->source, row->source)) { match = match->next; }
        if (!match || !equal(row, match)) { *failed = row->destination; return false; }
    }
    return true;
}
SourceOperationMaterialStatus source_operation_material_equal(SourceOperationMaterialReview *review) {
    return subset(&review->expected, review->plan->inputs.files, &review->failed_path)
        && subset(review->plan->inputs.files, &review->expected, &review->failed_path)
        ? SOURCE_OPERATION_MATERIAL_OK : SOURCE_OPERATION_MATERIAL_FILESET;
}
