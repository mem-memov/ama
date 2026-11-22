/** @file
 * @brief Own a finite immutable virtual source tree and measured reader failure controls.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_MODULE_VIEW_TEST_COMMON
#define AMA_MODULE_VIEW_TEST_COMMON
#include "../../common.c"
#include "../packages/support/io.h"
#include "source/module/view.h"
#include <sys/stat.h>
/** Exact nodes; the ignored dependency version exists only in its parent listing.
 * Any metadata/read request for that excluded version is a model violation.
 */
static const struct { const char *path; bool directory; const char *bytes; } nodes[] = {
    {"/virtual", true, NULL}, {"/virtual/packages", true, NULL},
    {"/virtual/packages/vendor", true, NULL}, {"/virtual/packages/vendor/pkg", true, NULL},
    {"/virtual/packages/vendor/pkg/v1.0.0", true, NULL},
    {"/virtual/packages/vendor/pkg/v1.0.0/src", true, NULL},
    {"/virtual/packages/vendor/pkg/v1.0.0/tests", true, NULL},
    {"/virtual/packages/vendor/pkg/v1.0.0/package.ini", false, ""},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state", false, "@!\n"},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state+", true, NULL},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state+/child", false, "@!\n"},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state.ini", false, "~type = //integer\n"},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state.c", false, "\xff opaque native bytes"},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state.h", false, "opaque native header"},
    {"/virtual/packages/vendor/pkg/v1.0.0/src/state.abi", false, ""},
    {"/virtual/packages/vendor/pkg/v1.0.0/tests/state", true, NULL},
    {"/virtual/packages/vendor/pkg/v1.0.0/tests/state/check", false, "@!\n"},
    {"/virtual/dependencies", true, NULL}, {"/virtual/dependencies/vendor", true, NULL},
    {"/virtual/dependencies/vendor/pkg", true, NULL},
    {"/standard", true, NULL}, {"/standard/integer", false, "@!\n"}
};
#define VIEW_NODE_COUNT (sizeof(nodes) / sizeof(nodes[0]))
/** All byte records belong to sources; callbacks only borrow them during read. */
typedef struct {
    struct stat directory, regular;
    const SourceText *texts[VIEW_NODE_COUNT], *invalid;
    size_t calls, reads, fail_at;
    bool source_alias, directory_alias, hide_state, hide_configuration, bad_utf8, null_read;
} ViewModel;
/** Count a callback independently of actual disk operations. */
static bool model_call(ViewModel *model) { return ++model->calls != model->fail_at; }
/** Return one exact known node, with typed absence for expected optional probes. */
static size_t node_index(ViewModel *model, const char *path) {
    for (size_t i = 0; i < VIEW_NODE_COUNT; i++) {
        if (!strcmp(nodes[i].path, path)) {
            if ((i == 8 && model->hide_state) || (i == 7 && model->hide_configuration)) { return VIEW_NODE_COUNT; }
            return i;
        }
    }
    return VIEW_NODE_COUNT;
}
/** Preserve owned request keys in failure diagnostics and exact alias classes. */
static ModuleStatus metadata(void *context, ModuleCatalog *catalog, const char *path, struct stat *output) {
    ViewModel *model = context;
    ModuleStatus status = MODULE_OK;
    if (!model_call(model)) { status = MODULE_IO_ERROR; }
    size_t node = node_index(model, path);
    if (status == MODULE_OK && node == VIEW_NODE_COUNT) { status = MODULE_MISSING; }
    if (status != MODULE_OK) { catalog->issue = (ModuleIssue){.status = status, .path = path}; return status; }
    *output = nodes[node].directory ? model->directory : model->regular;
    output->st_dev = 1; output->st_ino = (ino_t)(node + 1);
    if (node == 10 && model->source_alias) { output->st_ino = 9; }
    if (node == 15 && model->directory_alias) { output->st_ino = 10; }
    return MODULE_OK;
}
/** The common traversal must request the precise structural child it observed. */
static ModuleStatus child(void *context, ModuleCatalog *catalog, const char *parent,
    const char *name, const char *full, struct stat *output) {
    size_t length = strlen(parent);
    assert(!strncmp(parent, full, length) && full[length] == '/' && !strcmp(full + length + 1, name));
    return metadata(context, catalog, full, output);
}
/** Bytewise order is independent of model declaration or traversal order. */
static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}
/** Publish a complete sorted virtual directory view, never consulting the disk. */
static ModuleStatus entries(void *context, ModuleCatalog *catalog, const char *path,
    const char ***output, size_t *count) {
    ViewModel *model = context;
    if (!model_call(model)) { catalog->issue = (ModuleIssue){.status = MODULE_IO_ERROR, .path = path}; return MODULE_IO_ERROR; }
    size_t directory = node_index(model, path); assert(directory < VIEW_NODE_COUNT && nodes[directory].directory);
    const char **names = source_arena_allocate(&catalog->arena, (VIEW_NODE_COUNT + 1) * sizeof(*names));
    if (!names) { catalog->issue = (ModuleIssue){.status = MODULE_OUT_OF_MEMORY, .path = path}; return MODULE_OUT_OF_MEMORY; }
    size_t length = strlen(path), found = 0;
    for (size_t i = 0; i < VIEW_NODE_COUNT; i++) {
        if (i == 8 && model->hide_state) { continue; }
        if (!strncmp(nodes[i].path, path, length) && nodes[i].path[length] == '/' &&
            !strchr(nodes[i].path + length + 1, '/')) { names[found++] = nodes[i].path + length + 1; }
    }
    if (directory == 19) { names[found++] = "v1.0.0"; }
    qsort(names, found, sizeof(*names), compare_names); *output = names; *count = found;
    return MODULE_OK;
}
/** Borrow captured bytes; their old source path intentionally differs from the
 * virtual key so the test observes the catalog's deep-copy/path ownership boundary. */
static SourceStatus read_text(void *context, const char *path, const SourceText **output) {
    ViewModel *model = context;
    if (!model_call(model)) { return SOURCE_IO_ERROR; }
    size_t node = node_index(model, path); assert(node < VIEW_NODE_COUNT && !nodes[node].directory);
    model->reads++;
    *output = model->null_read ? NULL : node == 8 && model->bad_utf8 ? model->invalid : model->texts[node];
    return SOURCE_OK;
}
/** Construct all borrowed source snapshots before any allocation-failure scope. */
static void model_init(ViewModel *model) {
    memset(model, 0, sizeof(*model));
    assert(lstat(".", &model->directory) == 0 && S_ISDIR(model->directory.st_mode));
    assert(lstat("Makefile", &model->regular) == 0 && S_ISREG(model->regular.st_mode));
    for (size_t i = 0; i < VIEW_NODE_COUNT; i++) {
        if (!nodes[i].directory) {
            assert(source_manager_add(&sources, "original captured bytes", (const unsigned char *)nodes[i].bytes,
                strlen(nodes[i].bytes), &model->texts[i]) == SOURCE_OK);
        }
    }
    assert(source_manager_add(&sources, "invalid original", (const unsigned char *)"\xff", 1, &model->invalid) == SOURCE_OK);
}
/** Borrow the stable model through the complete catalog/companion operation. */
static ModuleCatalogReader reader_for(ViewModel *model) {
    return (ModuleCatalogReader){.tree = {.context = model, .root = metadata, .child = child, .entries = entries}, .read = read_text};
}
/** Build and complete lazy observations while measuring zero physical operations. */
static ModuleStatus prepare_catalog(ModuleCatalog *catalog, const ModuleCatalogReader *reader) {
    ModuleRoot roots[] = {{"/virtual", NULL, 0, MODULE_ROOT_PROJECT}, {"/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    fixture_io_begin(0);
    ModuleStatus status = module_catalog_build_view(catalog, roots, 2, reader);
    const ModuleSource *state = NULL, *child_source = NULL;
    if (status == MODULE_OK) { status = module_catalog_find(catalog, "/vendor/pkg/v1.0.0/src/state", &state); }
    if (status == MODULE_OK) { status = module_catalog_load_configuration(catalog, state); }
    if (status == MODULE_OK) { status = module_catalog_load_native(catalog, state); }
    if (status == MODULE_OK) { status = module_catalog_find(catalog, "/vendor/pkg/v1.0.0/src/state/child", &child_source); }
    if (status == MODULE_OK) { status = module_catalog_load_configuration(catalog, child_source); }
    assert(fixture_io_end() == 0);
    return status;
}
#endif
