/** @file
 * @brief Verify graph projection equality without allocation, recursive expansion or false byte certificates.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/graph/equal.h"
/** Retain independent source owners whose byte contents are outside graph metadata. */
static const SourceText *text(const char *path, const char *bytes) {
    const SourceText *value = NULL;
    assert(source_manager_add(&sources, path, (const unsigned char *)bytes, strlen(bytes), &value) == SOURCE_OK); return value;
}
/** Empty and independently owned equal metadata need no graph traversal or byte ownership. */
static void it_compares_empty_and_independent_cyclic_graph_projections_without_allocation(void) {
    setup_test();
    DeliverySourceDependency left_edge = {.target = "//a", .origin = {text("raw/\xff", "abc"), 0, 1}};
    DeliverySourceDependency right_edge = left_edge; right_edge.origin.source = text("raw/\xff", "different bytes");
    DeliverySourceDefinition left = {.identity = "//a", .kind = DELIVERY_SOURCE_CONCRETE, .dependencies = &left_edge};
    DeliverySourceDefinition right = left; right.dependencies = &right_edge;
    size_t before = allocation_count;
    assert(source_graph_equal(NULL, NULL) && source_graph_equal(&left, &right) && allocation_count == before);
    assert(!source_graph_equal(&left, NULL) && !source_graph_equal(NULL, &right)); destroy_test();
}
/** Definition identity/order/category and edge multiplicity are exact protocol facts. */
static void it_refuses_changed_definition_order_categories_and_edge_multiplicity(void) {
    setup_test(); DeliverySourceDependency edge = {.target = "//b", .origin = {text("main", "abc"), 0, 1}};
    DeliverySourceDefinition tail = {.identity = "//b", .kind = DELIVERY_SOURCE_STATIC};
    DeliverySourceDefinition left = {.next = &tail, .identity = "//a", .kind = DELIVERY_SOURCE_PARAMETERIZED, .dependencies = &edge};
    DeliverySourceDefinition right = left;
    right.kind = DELIVERY_SOURCE_CONCRETE; assert(!source_graph_equal(&left, &right)); right = left;
    right.identity = "//other"; assert(!source_graph_equal(&left, &right)); right = left;
    right.next = NULL; assert(!source_graph_equal(&left, &right)); right = left;
    assert(!source_graph_equal(&left, &tail));
    DeliverySourceDependency duplicate = edge; duplicate.next = &edge; right.dependencies = &duplicate;
    assert(!source_graph_equal(&left, &right)); destroy_test();
}
/** Every normalized edge field and original bounded occurrence affects equality. */
static void it_refuses_changed_targets_members_implicit_flags_paths_and_offsets(void) {
    setup_test(); DeliverySourceDependency edge = {.target = "//a", .member = "value", .origin = {text("main", "abc"), 0, 1}};
    DeliverySourceDependency changed = edge;
    DeliverySourceDefinition left = {.identity = "//a", .dependencies = &edge}, right = {.identity = "//a", .dependencies = &changed};
    changed.target = "//b"; assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.member = NULL; assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.member = "other"; assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.implicit = true; assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.origin.source = text("other", "abc"); assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.origin.start = 1; assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.origin.end = 2; assert(!source_graph_equal(&left, &right)); changed = edge;
    changed.origin.end = 4; assert(!source_graph_equal(&left, &right)); destroy_test();
}
/** Run metadata equality and its explicit authority/refusal boundaries. */
void run(void) {
    it_compares_empty_and_independent_cyclic_graph_projections_without_allocation();
    it_refuses_changed_definition_order_categories_and_edge_multiplicity();
    it_refuses_changed_targets_members_implicit_flags_paths_and_offsets();
}
/** Preserve generated output on assertion failure. */
int main(void) { run(); return 0; }
