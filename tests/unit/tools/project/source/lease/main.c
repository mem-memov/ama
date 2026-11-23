/** @file
 * @brief Exercise real hierarchical flock contention, stale roots and every owned failure prefix.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/transaction/lease/lease.h"
static SourceArena fixture;
static char *root_a, *root_b, *child;
/** Create disjoint/nested actual directories without changing compiler fixtures. */
static void begin(void) {
    setup_test(); source_arena_init(&fixture, sources.allocator);
    assert(mkdir("generated/a", 0700) == 0 && mkdir("generated/b", 0700) == 0 && mkdir("generated/a/child", 0700) == 0);
    root_a = realpath("generated/a", NULL); root_b = realpath("generated/b", NULL); child = realpath("generated/a/child", NULL);
    assert(root_a && root_b && child);
}
/** All lease descriptors retire before their directories and exact source owners. */
static void end(void) {
    assert(rmdir("generated/a/child") == 0 && rmdir("generated/a") == 0 && rmdir("generated/b") == 0);
    free(root_a); free(root_b); free(child); source_arena_destroy(&fixture); destroy_test();
}
/** Construct a genuinely validated root-only image with actual observed metadata;
 * no parser, whole-project discovery or invented module filename is needed. */
static void image(SourceCatalogImage *output, const char *const *paths, size_t count) {
    SourceCatalogImageEntry *entries = source_arena_allocate(&fixture, count * sizeof(*entries));
    ModuleRoot *roots = source_arena_allocate(&fixture, count * sizeof(*roots)); assert(entries && roots);
    for (size_t i = 0; i < count; i++) {
        entries[i] = (SourceCatalogImageEntry){.path = paths[i], .metadata_captured = true};
        assert(lstat(paths[i], &entries[i].metadata) == 0);
        roots[i] = (ModuleRoot){.path = paths[i], .kind = MODULE_ROOT_PROJECT};
    }
    SourceImageIssue issue;
    assert(source_catalog_image_prepare(output, entries, count, roots, count, &issue) == SOURCE_IMAGE_OK);
}
/** Shared ancestors permit disjoint roots, but exclusive roots conflict with
 * equal/nested roots; repeated configured roots acquire one inode lock. */
static void it_leases_disjoint_roots_and_refuses_overlapping_domains_without_mutation(void) {
    begin();
    SourceCatalogImage a, b, nested, repeated;
    const char *ap[] = {root_a}, *bp[] = {root_b}, *cp[] = {child}, *both[] = {child, root_a};
    image(&a, ap, 1); image(&b, bp, 1); image(&nested, cp, 1); image(&repeated, both, 2);
    SourceTransactionLease first, second; source_transaction_lease_init(&first, sources.allocator);
    assert(source_transaction_lease_acquire(&first, &a, (SourceLeaseControl){0}) == SOURCE_LEASE_OK && first.acquired);
    source_transaction_lease_init(&second, sources.allocator);
    assert(source_transaction_lease_acquire(&second, &b, (SourceLeaseControl){0}) == SOURCE_LEASE_OK);
    assert(source_transaction_lease_destroy(&second) == 0);
    source_transaction_lease_init(&second, sources.allocator);
    assert(source_transaction_lease_acquire(&second, &a, (SourceLeaseControl){0}) == SOURCE_LEASE_BUSY && !second.acquired);
    assert(source_transaction_lease_destroy(&second) == 0);
    source_transaction_lease_init(&second, sources.allocator);
    assert(source_transaction_lease_acquire(&second, &nested, (SourceLeaseControl){0}) == SOURCE_LEASE_BUSY);
    assert(source_transaction_lease_destroy(&second) == 0 && source_transaction_lease_destroy(&first) == 0);
    source_transaction_lease_init(&first, sources.allocator);
    assert(source_transaction_lease_acquire(&first, &repeated, (SourceLeaseControl){0}) == SOURCE_LEASE_OK);
    assert(source_transaction_lease_acquire(&first, &a, (SourceLeaseControl){0}) == SOURCE_LEASE_INVALID && first.acquired);
    assert(source_transaction_lease_destroy(&first) == 0);
    int parent = open("generated", O_RDONLY | O_DIRECTORY | O_CLOEXEC); assert(parent >= 0);
    assert(flock(parent, LOCK_EX | LOCK_NB) == 0);
    source_transaction_lease_init(&first, sources.allocator);
    assert(source_transaction_lease_acquire(&first, &a, (SourceLeaseControl){0}) == SOURCE_LEASE_BUSY);
    assert(source_transaction_lease_destroy(&first) == 0 && close(parent) == 0); end();
}
/** Original root inode/mode changes and substituted symbolic components cannot
 * become exclusive authority over a different physical tree. */
static void it_refuses_replaced_roots_and_symbolic_ancestors_before_acquisition(void) {
    begin(); SourceCatalogImage original; const char *paths[] = {child}; image(&original, paths, 1);
    assert(rename("generated/a/child", "generated/old") == 0 && mkdir("generated/a/child", 0700) == 0);
    SourceTransactionLease lease; source_transaction_lease_init(&lease, sources.allocator);
    assert(source_transaction_lease_acquire(&lease, &original, (SourceLeaseControl){0}) == SOURCE_LEASE_CHANGED && !lease.acquired);
    assert(source_transaction_lease_destroy(&lease) == 0);
    assert(rmdir("generated/a/child") == 0 && rename("generated/old", "generated/a/child") == 0);
    assert(rename("generated/a", "generated/old") == 0 && symlink("old", "generated/a") == 0);
    source_transaction_lease_init(&lease, sources.allocator);
    assert(source_transaction_lease_acquire(&lease, &original, (SourceLeaseControl){0}) == SOURCE_LEASE_IO && !lease.acquired);
    assert(source_transaction_lease_destroy(&lease) == 0);
    assert(unlink("generated/a") == 0 && rename("generated/old", "generated/a") == 0); end();
}
/** Controlled namespace substitution after locks proves the second namespace
 * validation, independently of initial original-root identity checks. */
static bool substitute(void *context, SourceLeaseOperation operation, const char *path) {
    bool *changed = context;
    if (operation == SOURCE_LEASE_VERIFY && !*changed && !strcmp(path, root_a)) {
        assert(rename("generated/a", "generated/old") == 0 && mkdir("generated/a", 0700) == 0);
        *changed = true;
    }
    return true;
}
/** A descriptor lease must not silently authorize a newly substituted pathname. */
static void it_revalidates_namespace_links_after_acquiring_locks(void) {
    begin(); SourceCatalogImage original; const char *paths[] = {root_a}; image(&original, paths, 1);
    SourceTransactionLease lease; source_transaction_lease_init(&lease, sources.allocator); bool changed = false;
    assert(source_transaction_lease_acquire(&lease, &original, (SourceLeaseControl){&changed, substitute}) == SOURCE_LEASE_CHANGED);
    assert(changed && !lease.acquired && source_transaction_lease_destroy(&lease) == 0);
    assert(rmdir("generated/a") == 0 && rename("generated/old", "generated/a") == 0); end();
}
/** Explicit synchronous cancellation enumerates operation boundaries, not time
 * or guessed scheduler interleavings. */
typedef struct { size_t calls, failure; } Cancellation;
/** Fail exactly one configured host-operation boundary. */
static bool permit(void *context, SourceLeaseOperation operation, const char *path) {
    Cancellation *state = context; (void)operation; assert(path); state->calls++;
    return state->calls != state->failure;
}
/** Every arena and cancellation ordinal closes all acquired prefix descriptors;
 * a fresh second owner can immediately acquire the same roots afterward. */
static void it_retires_every_lease_allocation_and_cancellation_prefix(void) {
    begin(); SourceCatalogImage original; const char *paths[] = {root_a, root_b}; image(&original, paths, 2);
    size_t allocations = 0, calls = 0, live = live_allocations;
    for (unsigned mode = 0; mode < 2; mode++) {
        size_t limit = 0;
        for (size_t failure = 0; !failure || failure <= limit; failure++) {
            SourceTransactionLease lease; source_transaction_lease_init(&lease, sources.allocator);
            Cancellation control = {.failure = mode ? failure : 0}; size_t before = allocation_count;
            if (!mode && failure) { failure_at = before + failure; }
            SourceLeaseStatus status = source_transaction_lease_acquire(&lease, &original, (SourceLeaseControl){&control, permit});
            if (!failure) { assert(status == SOURCE_LEASE_OK); allocations = allocation_count - before; calls = control.calls; limit = mode ? calls : allocations; assert(limit); }
            else { assert(status == (mode ? SOURCE_LEASE_CANCELLED : SOURCE_LEASE_MEMORY) && !lease.acquired); }
            failure_at = 0; assert(source_transaction_lease_destroy(&lease) == 0 && live_allocations == live);
            source_transaction_lease_init(&lease, sources.allocator);
            assert(source_transaction_lease_acquire(&lease, &original, (SourceLeaseControl){0}) == SOURCE_LEASE_OK);
            assert(source_transaction_lease_destroy(&lease) == 0 && live_allocations == live);
        }
    }
    SourceTransactionLease lease; source_transaction_lease_init(&lease, sources.allocator);
    assert(source_transaction_lease_acquire(&lease, NULL, (SourceLeaseControl){0}) == SOURCE_LEASE_INVALID);
    assert(source_transaction_lease_destroy(&lease) == 0); end();
}
/** Explicit cases exercise real kernel locks, resource ownership and namespace refusal. */
void run(void) {
    it_leases_disjoint_roots_and_refuses_overlapping_domains_without_mutation();
    it_refuses_replaced_roots_and_symbolic_ancestors_before_acquisition();
    it_retires_every_lease_allocation_and_cancellation_prefix();
    it_revalidates_namespace_links_after_acquiring_locks();
}
/** Return only after every fixture and lease owner has retired. */
int main(void) { run(); return 0; }
