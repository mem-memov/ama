#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "ownership.h"
#include "source/identity/hash/sha256.h"

const char *scaffold_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash != NULL ? slash + 1 : path;
}

bool scaffold_digest(NativeScaffold *scaffold, const SourceText *content, unsigned char digest[32]) {
    if ((uintmax_t)source_length(content) >= (UINT64_C(1) << 61)) {
        return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH, "Native output exceeds the SHA-256 input domain", source_path(content));
    }
    identity_sha256(source_bytes(content), source_length(content), digest);
    return true;
}

void scaffold_scope(const NativeScaffold *scaffold, unsigned char digest[32]) {
    const char *path = scaffold->selected->ama_path;
    identity_sha256((const unsigned char *)path, strlen(path), digest);
}

static int hexadecimal(unsigned char byte) {
    return byte >= '0' && byte <= '9' ? byte - '0' : byte >= 'a' && byte <= 'f' ? byte - 'a' + 10 : -1;
}

static bool record(NativeScaffold *scaffold, const unsigned char *bytes, size_t length) {
    if (length <= 65 || bytes[64] != ' ') {
        return false;
    }
    ScaffoldOwned *owned = scaffold_allocate(scaffold, sizeof(*owned));
    if (owned == NULL) {
        return false;
    }
    for (size_t index = 0; index < 32; index++) {
        int high = hexadecimal(bytes[index * 2]);
        int low = hexadecimal(bytes[index * 2 + 1]);
        if (high < 0 || low < 0) {
            return false;
        }
        owned->digest[index] = (unsigned char)(high * 16 + low);
    }
    for (size_t index = 65; index < length; index++) {
        unsigned char byte = bytes[index];
        if (!(byte >= 'a' && byte <= 'z') && !(byte >= 'A' && byte <= 'Z')
            && !(byte >= '0' && byte <= '9') && byte != '.' && byte != '-' && byte != '_') {
            return false;
        }
    }
    char *name = scaffold_allocate(scaffold, length - 65 + 1);
    if (name == NULL) {
        return false;
    }
    memcpy(name, bytes + 65, length - 65);
    if (name[0] == '.') {
        return false;
    }
    for (const ScaffoldOwned *other = scaffold->owned; other != NULL; other = other->next) {
        if (strcmp(other->name, name) == 0) {
            return false;
        }
    }
    owned->name = name;
    owned->next = scaffold->owned;
    scaffold->owned = owned;
    return true;
}

static bool scope_matches(NativeScaffold *scaffold, const unsigned char *bytes, size_t length) {
    if (length < 71 || memcmp(bytes, "scope ", 6) != 0 || bytes[70] != '\n') {
        return false;
    }
    unsigned char expected[32];
    scaffold_scope(scaffold, expected);
    bool equal = true;
    for (size_t index = 0; index < 32; index++) {
        int high = hexadecimal(bytes[6 + index * 2]);
        int low = hexadecimal(bytes[7 + index * 2]);
        if (high < 0 || low < 0) {
            return false;
        }
        equal = equal && expected[index] == (unsigned char)(high * 16 + low);
    }
    return equal || scaffold->options.overwrite;
}

bool scaffold_manifest_read(NativeScaffold *scaffold) {
    struct stat metadata;
    if (lstat(scaffold->manifest_path, &metadata) != 0) {
        return errno == ENOENT || scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot inspect the output manifest", scaffold->manifest_path);
    }
    if (!S_ISREG(metadata.st_mode)) {
        return scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_PATH, "Output manifest must be an ordinary file", scaffold->manifest_path);
    }
    const SourceText *text = NULL;
    SourceStatus loaded = source_manager_load(&scaffold->texts, scaffold->manifest_path, &text);
    if (loaded != SOURCE_OK) {
        return scaffold_error(scaffold, loaded == SOURCE_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_IO_FAILURE,
            "Cannot load the output manifest", scaffold->manifest_path);
    }
    const char header[] = "AMA-NATIVE-OUTPUTS 1\n";
    const unsigned char *bytes = source_bytes(text);
    size_t length = source_length(text), offset = sizeof(header) - 1;
    if (length < offset || memcmp(bytes, header, offset) != 0) {
        return false;
    }
    if (!scope_matches(scaffold, bytes + offset, length - offset)) {
        return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH,
            "Output scope differs or is malformed; choose a separate directory or explicitly overwrite a valid scope", scaffold->manifest_path);
    }
    offset += 71;
    while (offset < length) {
        size_t end = offset;
        while (end < length && bytes[end] != '\n') {
            end++;
        }
        if (end == length || !record(scaffold, bytes + offset, end - offset)) {
            return false;
        }
        offset = end + 1;
    }
    return true;
}
