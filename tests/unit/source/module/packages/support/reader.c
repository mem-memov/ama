/**
 * @file
 * @brief Verify reader-driven selection against an independent finite virtual directory tree.
 * The model has no files on disk. Linker I/O counters prove selection uses only
 * supplied callbacks, and each callback can refuse without publishing output.
 */
#include "source/module/packages/reader.h"

/** Caller-owned finite model; no pointer into it may escape into retained errors. */
typedef struct {
    struct stat directory;
    size_t calls;
    size_t fail_at;
} PackageReaderModel;

/** Exact virtual nodes. The ignored dependency version has no metadata node:
 * its presence in the parent listing must be enough to skip it before probing.
 */
static const char *const model_paths[] = {
    "/virtual", "/virtual/packages", "/virtual/packages/vendor",
    "/virtual/packages/vendor/pkg", "/virtual/packages/vendor/pkg/v1.0.0",
    "/virtual/dependencies", "/virtual/dependencies/vendor", "/virtual/dependencies/vendor/pkg"
};

/** Count one synchronous callback and retain an owned path when it refuses. */
static bool model_call(PackageReaderModel *model, ModuleCatalog *owner, const char *path) {
    model->calls++;
    if (model->calls != model->fail_at) { return true; }
    owner->issue = (ModuleIssue){.status = MODULE_IO_ERROR, .path = path};
    return false;
}

/** Find an exact declared node; unknown metadata requests are model violations. */
static size_t model_node(const char *path) {
    for (size_t index = 0; index < sizeof(model_paths) / sizeof(*model_paths); index++) {
        if (strcmp(path, model_paths[index]) == 0) { return index; }
    }
    assert(false);
    return 0;
}

/** Publish stable directory identity only after the modeled metadata read succeeds. */
static ModuleStatus model_metadata(void *context, ModuleCatalog *owner,
    const char *path, struct stat *metadata) {
    PackageReaderModel *model = context;
    if (!model_call(model, owner, path)) { return MODULE_IO_ERROR; }
    size_t node = model_node(path);
    *metadata = model->directory;
    metadata->st_dev = 1;
    metadata->st_ino = (ino_t)(node + 1);
    return MODULE_OK;
}

/** Check the selector's exact structural child join before reading virtual metadata. */
static ModuleStatus model_child(void *context, ModuleCatalog *owner,
    const char *parent, const char *name, const char *full, struct stat *metadata) {
    size_t width = strlen(parent);
    assert(strncmp(parent, full, width) == 0 && full[width] == '/');
    assert(strcmp(full + width + 1, name) == 0);
    return model_metadata(context, owner, full, metadata);
}

/** Supply one exact sorted nonempty child set; version contents are never listed. */
static ModuleStatus model_entries(void *context, ModuleCatalog *owner,
    const char *path, const char ***names, size_t *count) {
    PackageReaderModel *model = context;
    if (!model_call(model, owner, path)) { return MODULE_IO_ERROR; }
    size_t node = model_node(path);
    static const char *vendor[] = {"vendor"};
    static const char *package[] = {"pkg"};
    static const char *version[] = {"v1.0.0"};
    assert(node != 0 && node != 4);
    *names = node == 1 || node == 5 ? vendor : node == 2 || node == 6 ? package : version;
    *count = 1;
    return MODULE_OK;
}

/** One finite view selects the local complete version with no physical I/O;
 * every callback failure preserves output and releases all selection allocations.
 */
static void it_selects_virtual_versions_and_unwinds_every_reader_callback_failure(void) {
    struct stat directory;
    assert(lstat(".", &directory) == 0 && S_ISDIR(directory.st_mode));
    size_t calls = 0;
    for (size_t failure = 0; failure <= calls; failure++) {
        setup_catalog();
        PackageReaderModel model = {.directory = directory, .fail_at = failure};
        ModulePackageReader reader = {.context = &model, .root = model_metadata,
            .child = model_child, .entries = model_entries};
        char root_path[] = "/virtual";
        ModuleRoot root = {.path = root_path, .kind = MODULE_ROOT_PROJECT};
        ModulePackage sentinel, *output = &sentinel;
        fixture_io_begin(1);
        ModuleStatus status = module_packages_select_view(&catalog, &root, 1, &reader, &output);
        assert(fixture_io_end() == 0);
        if (failure == 0) {
            assert(status == MODULE_OK && output != NULL && output->next == NULL);
            assert(output->local && strcmp(output->logical, "/vendor/pkg/v1.0.0") == 0);
            assert(strcmp(output->physical, model_paths[4]) == 0);
            calls = model.calls;
            assert(calls != 0);
        } else {
            assert(status == MODULE_IO_ERROR && model.calls == failure && output == &sentinel);
            root_path[1] = 'X';
            assert(catalog.issue.path != NULL && strstr(catalog.issue.path, "/virtual") == catalog.issue.path);
        }
        destroy_catalog();
    }
}

/** Incomplete reader interfaces and null roots fail before callbacks/output/allocation. */
static void it_requires_a_complete_reader_before_selecting_any_versions(void) {
    setup_catalog();
    ModulePackage sentinel, *output = &sentinel;
    ModulePackageReader reader = {0};
    size_t allocations = allocation_count;
    assert(module_packages_select_view(&catalog, NULL, 0, NULL, &output) == MODULE_INVALID_PATH);
    assert(module_packages_select_view(&catalog, NULL, 0, &reader, &output) == MODULE_INVALID_PATH);
    reader.root = model_metadata;
    assert(module_packages_select_view(&catalog, NULL, 0, &reader, &output) == MODULE_INVALID_PATH);
    reader.child = model_child;
    assert(module_packages_select_view(&catalog, NULL, 0, &reader, &output) == MODULE_INVALID_PATH);
    reader.entries = model_entries;
    ModuleRoot invalid = {.kind = MODULE_ROOT_PROJECT};
    assert(module_packages_select_view(&catalog, &invalid, 1, &reader, &output) == MODULE_INVALID_PATH);
    assert(output == &sentinel && allocation_count == allocations);
    assert(module_packages_select_view(&catalog, NULL, 0, &reader, &output) == MODULE_OK && output == NULL);
    destroy_catalog();
}
