/** @file
 * @brief Own isolated physical fixtures and counted production allocation authority for relocation tests.
 * Fixture path strings use a separate system arena so fail-at-N measures production work alone.
 */
#ifndef AMA_RELOCATION_TEST_COMMON
#define AMA_RELOCATION_TEST_COMMON
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "../../source/common.c"
#include "tools/relocation/relocation.h"

static SourceArena fixture_paths;
static const char *fixture_base;
static RelocationPreview preview;

/** Keep each fixture path stable through production destruction and retained diagnostic checks. */
static const char *path(const char *relative) {
    const char *result = source_arena_join(&fixture_paths,fixture_base,"/",relative);
    assert(result != NULL);
    return result;
}
/** Create one explicitly ordered fixture directory; unrelated failures are never accepted. */
static void folder(const char *relative) {
    int result = mkdir(path(relative),0700);
    assert(result == 0 || errno == EEXIST);
}
/** Write exact opaque or source bytes without applying any language formatting. */
static void bytes(const char *relative, const void *data, size_t length) {
    FILE *stream = fopen(path(relative),"wb");
    assert(stream != NULL);
    assert(fwrite(data,1,length,stream) == length);
    assert(fclose(stream) == 0);
}
/** Ordinary textual fixtures retain exact supplied whitespace and line endings. */
static void file(const char *relative, const char *text) { bytes(relative,text,strlen(text)); }
/** Prepare one explicitly named local version with required source/test areas.
 * Both arguments are fixture-relative paths/names chosen by the case; no file
 * content or basename decides a package identity.
 */
static void package(const char *root, const char *name) {
    folder(root);
    const char *storage = source_arena_join(&fixture_paths, root, "/", "packages");
    const char *vendor = source_arena_join(&fixture_paths, storage, "/", "test");
    const char *group = source_arena_join(&fixture_paths, vendor, "/", name);
    const char *version = source_arena_join(&fixture_paths, group, "/", "v1.0.0");
    assert(storage && vendor && group && version);
    folder(storage); folder(vendor); folder(group); folder(version);
    const char *source = source_arena_join(&fixture_paths, version, "/", "src");
    const char *tests = source_arena_join(&fixture_paths, version, "/", "tests");
    const char *config = source_arena_join(&fixture_paths, version, "/", "package.ini");
    assert(source && tests && config);
    folder(source); folder(tests); file(config, "");
}

/** Begin independent filesystem and counted-owner fixtures; no previous case is reused as an oracle. */
static void setup(const char *name) {
    setup_test();
    source_arena_init(&fixture_paths,source_system_allocator());
    fixture_base = source_arena_join(&fixture_paths,"generated","/",name);
    assert(fixture_base != NULL);
    assert(mkdir(fixture_base,0700) == 0);
    relocation_preview_init(&preview,sources.allocator);
}
/** Release the production owner before its input path fixtures and allocator authority. */
static void finish(void) {
    relocation_preview_destroy(&preview);
    source_arena_destroy(&fixture_paths);
    destroy_test();
}
/** Reuse only stable physical fixtures; each preparation gets a fresh independent owner. */
static void reset_preview(void) {
    relocation_preview_destroy(&preview);
    relocation_preview_init(&preview,sources.allocator);
}
/** Select a complete original identity from the immutable preview, including inactive drafts. */
static const RelocationModule *module(const char *identity) {
    for (const RelocationModule *item = preview.modules; item != NULL; item = item->next) {
        if (item->before_identity != NULL && strcmp(item->before_identity,identity) == 0) { return item; }
    }
    return NULL;
}
/** Prove preview did not create its destination and left its actual source directory present. */
static void untouched(const RelocationRequest *request) {
    struct stat metadata;
    assert(lstat(request->source,&metadata) == 0 && S_ISDIR(metadata.st_mode));
    assert(lstat(request->destination,&metadata) != 0 && errno == ENOENT);
}
#endif
