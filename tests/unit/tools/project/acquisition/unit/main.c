/** @file
 * @brief Verify complete physical unit bytes, topology exclusions and atomic failure outputs.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../closure/common.c"
#include "../../../../../../tools/project/acquisition/closure/unit/unit.h"
#include <errno.h>
#include <unistd.h>
/** Prepare only test-owned stable directories, failing unexpected OS errors. */
static void directory(const char *path) {
    int status = mkdir(path, 0700); assert(status == 0 || errno == EEXIST);
}
/** Write all bytes including embedded zero; neither C nor Ama interprets assets. */
static void file(const char *path, const unsigned char *bytes, size_t length) {
    FILE *stream = fopen(path, "wb"); assert(stream);
    size_t written = fwrite(bytes, 1, length, stream); int closed = fclose(stream);
    assert(written == length && closed == 0);
}
/** Shared shape includes core/arbitrary companions, an unused child and empty/hidden assets. */
static void prepare(void) {
    directory("generated/unit"); directory("generated/unit/main+");
    directory("generated/unit/main+/empty"); directory("generated/unit/main+/nested");
    static const unsigned char binary[] = {0, 255, '\r', '\n'};
    file("generated/unit/main", (const unsigned char *)"@!\n", 3);
    file("generated/unit/main.c", binary, sizeof(binary));
    file("generated/unit/main.h", (const unsigned char *)"", 0);
    file("generated/unit/main.abi", (const unsigned char *)"+ construct\n", 12);
    file("generated/unit/main.cmake", (const unsigned char *)"message(FATAL_ERROR stop)\n", 26);
    file("generated/unit/main.extra", binary, sizeof(binary));
    file("generated/unit/main.", binary, sizeof(binary));
    file("generated/unit/other", binary, sizeof(binary));
    file("generated/unit/main+/unused", (const unsigned char *)"@!\n", 3);
    file("generated/unit/main+/.hidden", binary, sizeof(binary));
    file("generated/unit/main+/nested/raw", binary, sizeof(binary));
}
/** Compare independently listed membership and byte observations in strict order. */
static void it_retains_all_active_structural_inputs_and_no_neighbor_units(void) {
    prepare(); begin();
    const AcquisitionUnitEntry *entries = NULL; const char *failed = "unchanged";
    SourceStatus status = acquisition_unit_capture(&results, &sources, "generated/unit", "main",
        ACQUISITION_ACTIVE, &entries, &failed);
    assert(status == SOURCE_OK && !failed);
    static const char *const expected[] = {"main", "main+", "main+/.hidden", "main+/empty", "main+/nested", "main+/nested/raw", "main+/unused", "main.abi", "main.c", "main.cmake", "main.extra", "main.h"};
    const AcquisitionUnitEntry *item = entries;
    for (size_t index = 0; index < sizeof(expected) / sizeof(expected[0]); index++) {
        assert(item && !strcmp(item->file.path + strlen("generated/unit/"), expected[index]));
        bool is_directory = index == 1 || index == 3 || index == 4;
        assert(item->directory == is_directory && item->file.state == DELIVERY_FILE_PRESENT);
        if (is_directory) { assert(!item->file.content); }
        else { assert(item->file.content); }
        if (index == 8 || index == 10) {
            static const unsigned char bytes[] = {0, 255, '\r', '\n'};
            assert(source_length(item->file.content) == sizeof(bytes));
            assert(!memcmp(source_bytes(item->file.content), bytes, sizeof(bytes)));
        }
        item = item->next;
    }
    assert(!item); finish();
}
/** Context ancestors keep their main/companions, never enter a defective unused subtree. */
static void it_retains_context_without_entering_its_subtree_and_rejects_links_when_active(void) {
    prepare(); int linked = symlink("../main+", "generated/unit/main+/cycle"); assert(linked == 0);
    begin(); const AcquisitionUnitEntry *entries = NULL; const char *failed = NULL;
    SourceStatus status = acquisition_unit_capture(&results, &sources, "generated/unit", "main",
        ACQUISITION_CONTEXT, &entries, &failed); assert(status == SOURCE_OK);
    size_t count = 0;
    for (const AcquisitionUnitEntry *item = entries; item; item = item->next) {
        assert(!item->directory && !strchr(item->file.path, '+')); count++;
    }
    assert(count == 6);
    AcquisitionUnitEntry sentinel = {0}; entries = &sentinel;
    status = acquisition_unit_capture(&results, &sources, "generated/unit", "main",
        ACQUISITION_ACTIVE, &entries, &failed);
    assert(status == SOURCE_IO_ERROR && entries == &sentinel && failed && strstr(failed, "/cycle"));
    int removed = unlink("generated/unit/main+/cycle"); assert(removed == 0); finish();
}
/** Missing main is fatal, absent subtree is healthy, and directories cannot replace companions. */
static void it_distinguishes_absent_optional_subtrees_from_invalid_required_inputs(void) {
    prepare(); begin(); const AcquisitionUnitEntry *entries = NULL; const char *failed = NULL;
    file("generated/unit/plain", (const unsigned char *)"@!\n", 3);
    SourceStatus status = acquisition_unit_capture(&results, &sources, "generated/unit", "plain",
        ACQUISITION_ACTIVE, &entries, &failed);
    assert(status == SOURCE_OK && entries && !entries->next && !entries->directory);
    const AcquisitionUnitEntry *previous = entries;
    status = acquisition_unit_capture(&results, &sources, "generated/unit", "missing",
        ACQUISITION_ACTIVE, &entries, &failed); assert(status == SOURCE_IO_ERROR && entries == previous && failed);
    directory("generated/unit/plain.data");
    status = acquisition_unit_capture(&results, &sources, "generated/unit", "plain",
        ACQUISITION_CONTEXT, &entries, &failed); assert(status == SOURCE_IO_ERROR && entries == previous && strstr(failed, "plain.data"));
    int removed = rmdir("generated/unit/plain.data"); assert(removed == 0);
    finish();
}
/** Every capture allocation, including enumeration buffers and text bytes, can fail
 * without publishing a partial membership; all scratch is released by the owners. */
static void it_retires_every_failed_capture_allocation_without_partial_output(void) {
    prepare(); size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        begin(); AcquisitionUnitEntry sentinel = {0}; const AcquisitionUnitEntry *entries = &sentinel;
        const char *failed = NULL; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceStatus status = acquisition_unit_capture(&results, &sources, "generated/unit", "main",
            ACQUISITION_ACTIVE, &entries, &failed);
        if (!failure) { assert(status == SOURCE_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_OUT_OF_MEMORY && entries == &sentinel); failure_at = 0; }
        finish();
    }
}
/** Required directory capture retains the root and complete tree, but missing
 * or non-directory roots cannot count as associated test inputs. */
static void it_captures_required_directories_and_refuses_missing_or_non_directory_roots(void) {
    prepare(); begin(); const AcquisitionUnitEntry *entries = NULL; const char *failed = NULL;
    SourceStatus status = acquisition_directory_capture(&results, &sources, "generated/unit", "main+", &entries, &failed);
    assert(status == SOURCE_OK && entries && entries->directory && !failed);
    size_t count = 0;
    for (const AcquisitionUnitEntry *item = entries; item; item = item->next) { count++; }
    assert(count == 6);
    const AcquisitionUnitEntry *previous = entries;
    status = acquisition_directory_capture(&results, &sources, "generated/unit", "absent", &entries, &failed);
    assert(status == SOURCE_IO_ERROR && entries == previous && failed && strstr(failed, "/absent"));
    status = acquisition_directory_capture(&results, &sources, "generated/unit", "main", &entries, &failed);
    assert(status == SOURCE_IO_ERROR && entries == previous);
    status = acquisition_directory_capture(&results, &sources, "generated/unit", "../escape", &entries, &failed);
    assert(status == SOURCE_INVALID_ARGUMENT && entries == previous);
    finish();
}
/** Each directory capture allocation may fail without publishing a partial tree. */
static void it_retires_every_required_directory_allocation_failure(void) {
    prepare(); size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        begin(); AcquisitionUnitEntry sentinel = {0}; const AcquisitionUnitEntry *entries = &sentinel;
        const char *failed = NULL; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceStatus status = acquisition_directory_capture(&results, &sources, "generated/unit", "main+", &entries, &failed);
        if (!failure) { assert(status == SOURCE_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_OUT_OF_MEMORY && entries == &sentinel); failure_at = 0; }
        finish();
    }
}
/** Run exact delivery membership, topology, I/O and exhaustive allocation partitions. */
void run(void) {
    it_retains_all_active_structural_inputs_and_no_neighbor_units();
    it_retains_context_without_entering_its_subtree_and_rejects_links_when_active();
    it_distinguishes_absent_optional_subtrees_from_invalid_required_inputs();
    it_retires_every_failed_capture_allocation_without_partial_output();
    it_captures_required_directories_and_refuses_missing_or_non_directory_roots();
    it_retires_every_required_directory_allocation_failure();
}
/** Finish after all operation owners report zero live allocations. */
int main(void) { run(); return 0; }
