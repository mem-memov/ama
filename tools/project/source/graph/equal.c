/** @file
 * @brief Reject changed graph categories, edges and original source sites with finite exact scans.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves normalized member and implicit-edge distinctions;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps metadata equality separate from captured-byte authority.
 */
#include <string.h>
#include "equal.h"
/** Nullable member values are distinct from empty or present names under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static bool name(const char *left, const char *right) {
    return left && right ? !strcmp(left, right) : left == right;
}
/** Byte paths and bounded original offsets identify one occurrence under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool site(SourceSpan left, SourceSpan right) {
    return source_span_valid(left) && source_span_valid(right) && left.start == right.start && left.end == right.end
        && name(source_path(left.source), source_path(right.source));
}
/** Paired traversal preserves duplicate edges and their original source order
 * under [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static bool edges(const DeliverySourceDependency *left, const DeliverySourceDependency *right) {
    while (left && right) {
        if (!name(left->target, right->target) || left->implicit != right->implicit
            || !name(left->member, right->member) || !site(left->origin, right->origin)) { return false; }
        left = left->next; right = right->next;
    }
    return !left && !right;
}
bool source_graph_equal(const DeliverySourceDefinition *left, const DeliverySourceDefinition *right) {
    while (left && right) {
        if (!name(left->identity, right->identity) || left->kind != right->kind || !edges(left->dependencies, right->dependencies)) { return false; }
        left = left->next; right = right->next;
    }
    return !left && !right;
}
