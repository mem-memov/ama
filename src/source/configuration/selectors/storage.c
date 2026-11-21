/**
 * @file
 * @brief Copy expanded selector components into immutable scalar storage before publication.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) retains exact literals
 * and star placement. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * owns partial allocations; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks component/scalar bounds. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * preserves origins without retaining the alias table.
 */
#include <stddef.h>
#include "internal.h"
#include "source/unicode/unicode.h"

/** Adjacent parts share the immutable selector's arena lifetime.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) uses real flexible-member offsets.
 */
typedef struct { DependencySelector selector; DependencySelectorPart parts[]; } SelectorStorage;

DependencySelectorStatus selector_store(SourceArena *arena, SelectorCursor input, SelectorMeasure measure,
    SourceSpan expression, const DependencySelector **output, SourceIssue *issue) {
    size_t count = measure.count, bytes = measure.bytes;
    size_t header = offsetof(SelectorStorage, parts);
    if (count > (SIZE_MAX - header) / sizeof(DependencySelectorPart)
        || bytes > SIZE_MAX / sizeof(uint32_t)) { goto limit; }
    size_t size = header + count * sizeof(DependencySelectorPart);
    if (size < sizeof(SelectorStorage)) { size = sizeof(SelectorStorage); }
    SelectorStorage *storage = source_arena_allocate(arena, size);
    uint32_t *scalars = storage == NULL ? NULL : source_arena_allocate(arena, bytes * sizeof(uint32_t));
    if (scalars == NULL) {
        return selector_failure(issue, expression, DEPENDENCY_SELECTOR_OUT_OF_MEMORY,
            "Dependency pattern allocation failed", "available component and scalar storage");
    }
    SelectorCursor cursor = input;
    ModulePathPart part;
    size_t index = 0, used = 0;
    while (selector_next(&cursor, &part)) {
        size_t start = used;
        for (size_t offset = 0; offset < part.length;) {
            size_t width;
            if (!utf8_decode(part.bytes + offset, part.length - offset, &scalars[used], &width)) {
                return selector_failure(issue, part.origin, DEPENDENCY_SELECTOR_INVALID,
                    "Invalid selector component encoding", "complete UTF-8 component scalars");
            }
            used++;
            offset += width;
        }
        storage->parts[index++] = (DependencySelectorPart){scalars + start, used - start,
            part.origin, used - start == 1 && scalars[start] == '*'};
    }
    storage->selector = (DependencySelector){storage->parts, count, expression};
    *output = &storage->selector;
    return DEPENDENCY_SELECTOR_OK;
limit:
    return selector_failure(issue, expression, DEPENDENCY_SELECTOR_LIMIT,
        "Dependency pattern size is not representable", "representable component and scalar counts");
}
