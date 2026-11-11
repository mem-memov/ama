#include <string.h>
#include "ownership.h"

static const ScaffoldArtifact *generated(const NativeScaffold *scaffold, const char *name) {
    for (const ScaffoldArtifact *artifact = scaffold->first; artifact != NULL; artifact = artifact->next) {
        if (!artifact->authored && strcmp(scaffold_basename(artifact->path), name) == 0) {
            return artifact;
        }
    }
    return NULL;
}

static bool row(FILE *output, const char *name, const unsigned char digest[32]) {
    for (size_t index = 0; index < 32; index++) {
        if (fprintf(output, "%02x", (unsigned)digest[index]) < 0) {
            return false;
        }
    }
    return fprintf(output, " %s\n", name) >= 0;
}

static bool emit(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("AMA-NATIVE-OUTPUTS 1\n", output) == EOF) {
        return false;
    }
    unsigned char scope[32];
    scaffold_scope(scaffold, scope);
    if (fputs("scope ", output) == EOF) {
        return false;
    }
    for (size_t index = 0; index < 32; index++) {
        if (fprintf(output, "%02x", (unsigned)scope[index]) < 0) {
            return false;
        }
    }
    if (fputc('\n', output) == EOF) {
        return false;
    }
    for (const ScaffoldOwned *owned = scaffold->owned; owned != NULL; owned = owned->next) {
        if (generated(scaffold, owned->name) == NULL && !row(output, owned->name, owned->digest)) {
            return false;
        }
    }
    for (const ScaffoldArtifact *artifact = scaffold->first; artifact != NULL; artifact = artifact->next) {
        unsigned char digest[32];
        if (!artifact->authored && (!scaffold_digest(scaffold, artifact->content, digest)
                || !row(output, scaffold_basename(artifact->path), digest))) {
            return false;
        }
    }
    return true;
}

bool scaffold_manifest(NativeScaffold *scaffold) {
    scaffold->manifest_path = scaffold_join(scaffold, scaffold->directory, "/", ".ama-native-outputs");
    if (scaffold->manifest_path == NULL || !scaffold_manifest_read(scaffold)) {
        return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH,
            "Invalid output manifest; repair it or choose a fresh output directory", scaffold->manifest_path);
    }
    return scaffold_artifact(scaffold, scaffold->manifest_path, false, emit, NULL);
}

bool scaffold_owned_output(NativeScaffold *scaffold, const ScaffoldArtifact *artifact) {
    if (artifact->authored) {
        return false;
    }
    if (strcmp(artifact->path, scaffold->manifest_path) == 0) {
        return true;
    }
    const char *name = scaffold_basename(artifact->path);
    for (const ScaffoldOwned *owned = scaffold->owned; owned != NULL; owned = owned->next) {
        if (strcmp(owned->name, name) != 0) {
            continue;
        }
        const SourceText *current = NULL;
        SourceStatus loaded = source_manager_load(&scaffold->texts, artifact->path, &current);
        if (loaded != SOURCE_OK) {
            return scaffold_error(scaffold, loaded == SOURCE_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_IO_FAILURE,
                "Cannot validate owned output contents", artifact->path);
        }
        unsigned char digest[32];
        return scaffold_digest(scaffold, current, digest) && memcmp(digest, owned->digest, sizeof(digest)) == 0;
    }
    return false;
}
