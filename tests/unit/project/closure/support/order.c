/** @file
 * @brief Exhaust stable physical-order sorting independently from AST collection shape.
 */
#include "project/dependencies/discover/internal.h"
/** Compare both unique and equal-position requests with exact monotonic/stability laws. */
static void check_dependency_permutation(const size_t *order, const SourceText *source) {
    for (size_t grouped = 0; grouped < 2; grouped++) {
        DependencyPath nodes[7];
        for (size_t index = 0; index < 7; index++) {
            size_t offset = grouped ? order[index] / 2 : order[index];
            nodes[index] = (DependencyPath){.next = index + 1 == 7 ? NULL : &nodes[index + 1], .path = {source, offset, offset + 1}};
        }
        DependencyPaths paths = {nodes, 7};
        size_t allocations = allocation_count;
        project_sort_paths(&paths);
        assert(allocation_count == allocations);
        const DependencyPath *previous = NULL, *node = paths.first;
        size_t count = 0;
        bool seen[7] = {false};
        for (; node != NULL; node = node->next) {
            size_t identity = (size_t)(node - nodes);
            assert(identity < 7 && !seen[identity]);
            seen[identity] = true;
            if (previous != NULL) {
                assert(previous->path.start <= node->path.start);
                if (previous->path.start == node->path.start) { assert(previous < node); }
            }
            if (!grouped) { assert(node->path.start == count); }
            previous = node;
            count++;
        }
        assert(count == 7 && paths.count == 7);
    }
}
/** Enumerate seven-element permutations with a fixed test-side recursion bound. */
static size_t dependency_orders(size_t *order, size_t count, unsigned mask, const SourceText *source) {
    if (count == 7) { check_dependency_permutation(order, source); return 1; }
    size_t visited = 0;
    for (size_t index = 0; index < 7; index++) {
        if ((mask & (1u << index)) == 0) {
            order[count] = index;
            visited += dependency_orders(order, count + 1, mask | (1u << index), source);
        }
    }
    return visited;
}
/** Every permutation and equal-key grouping preserves exactly the original request multiset and stability. */
static void it_sorts_every_seven_request_permutation_stably_without_allocation(void) {
    setup_project();
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "requests", (const unsigned char *)"abcdefg", 7, &source);
    assert(added == SOURCE_OK);
    size_t order[7];
    size_t count = dependency_orders(order, 0, 0, source);
    assert(count == 5040);
    DependencyPaths empty = {0};
    project_sort_paths(&empty);
    assert(empty.first == NULL && empty.count == 0);
    destroy_project();
}
