#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "native/scaffold/internal.h"

static bool directories(NativeScaffold *scaffold) {
    char *path = source_arena_copy(&scaffold->arena, scaffold->directory);
    if (path == NULL) {
        return scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "Cannot retain output directory", NULL);
    }
    size_t length = strlen(path);
    for (size_t index = 1; index <= length; index++) {
        if (path[index] != '/' && path[index] != 0) {
            continue;
        }
        char saved = path[index];
        path[index] = 0;
        struct stat metadata;
        if (lstat(path, &metadata) != 0) {
            if (errno != ENOENT || mkdir(path, 0700) != 0 || lstat(path, &metadata) != 0) {
                return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot create output directory", scaffold->directory);
            }
        }
        if (!S_ISDIR(metadata.st_mode)) {
            return scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_PATH,
                "Output directory components must be ordinary directories", scaffold->directory);
        }
        path[index] = saved;
    }
    return true;
}

static bool preflight(NativeScaffold *scaffold) {
    for (ScaffoldArtifact *artifact = scaffold->first; artifact != NULL; artifact = artifact->next) {
        struct stat metadata;
        if (lstat(artifact->path, &metadata) != 0) {
            if (errno != ENOENT) {
                return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot inspect native output", artifact->path);
            }
            continue;
        }
        artifact->existed = true;
        if (!S_ISREG(metadata.st_mode)) {
            return scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_PATH,
                "Native output cannot replace a symlink or nonregular file", artifact->path);
        }
        if (!scaffold->options.overwrite && !scaffold_owned_output(scaffold, artifact)) {
            return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH,
                "Native output conflicts with an existing file; explicit overwrite is required", artifact->path);
        }
    }
    return true;
}

static char *temporary(NativeScaffold *scaffold, const char *path, const char *suffix, int *descriptor) {
    char *name = source_arena_join(&scaffold->arena, path, "", suffix);
    if (name == NULL) {
        scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "Cannot allocate a staging path", path);
        return NULL;
    }
    *descriptor = mkstemp(name);
    if (*descriptor < 0) {
        scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot create a staging file", path);
        return NULL;
    }
    return name;
}

static bool stage(NativeScaffold *scaffold, ScaffoldArtifact *artifact) {
    int descriptor = -1;
    artifact->stage = temporary(scaffold, artifact->path, ".ama-stage-XXXXXX", &descriptor);
    if (artifact->stage == NULL) {
        return false;
    }
    FILE *output = fdopen(descriptor, "wb");
    if (output == NULL) {
        close(descriptor);
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot open a staging stream", artifact->path);
    }
    if (artifact->executable && fchmod(descriptor,0700)!=0) {
        fclose(output);
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot set executable permissions", artifact->path);
    }
    size_t length = source_length(artifact->content);
    bool written = fwrite(source_bytes(artifact->content), 1, length, output) == length;
    int closed = fclose(output);
    if (!written || closed != 0) {
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot finish a staging stream", artifact->path);
    }
    if (artifact->existed) {
        char *backup = temporary(scaffold, artifact->path, ".ama-backup-XXXXXX", &descriptor);
        if (backup == NULL) {
            return false;
        }
        artifact->backup = backup;
        int closed_backup = close(descriptor);
        int removed = unlink(backup);
        if (closed_backup != 0 || removed != 0 || link(artifact->path, backup) != 0) {
            return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot retain the existing output for rollback", artifact->path);
        }
        artifact->backup = backup;
    }
    return true;
}

static bool publish_one(NativeScaffold *scaffold, ScaffoldArtifact *artifact) {
    if (artifact->existed) {
        if (rename(artifact->stage, artifact->path) != 0) {
            return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot replace native output", artifact->path);
        }
    } else if (link(artifact->stage, artifact->path) != 0) {
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot publish a new native output", artifact->path);
    }
    artifact->published = true;
    return true;
}

static void cleanup(NativeScaffold *scaffold, bool rollback) {
    for (ScaffoldArtifact *artifact = scaffold->first; artifact != NULL; artifact = artifact->next) {
        if (rollback && artifact->published) {
            int restored = artifact->existed ? rename(artifact->backup, artifact->path) : unlink(artifact->path);
            if (restored != 0) {
                scaffold->error.cause = "Output rollback also failed; inspect the affected files and retained backups";
                continue;
            }
            artifact->published = false;
        }
        if (artifact->stage != NULL) {
            (void)unlink(artifact->stage);
        }
        if (artifact->backup != NULL) {
            (void)unlink(artifact->backup);
        }
    }
}

static bool transaction(NativeScaffold *scaffold) {
    if (!scaffold_manifest(scaffold) || !preflight(scaffold)) {
        return false;
    }
    bool complete = true;
    for (ScaffoldArtifact *artifact = scaffold->first; complete && artifact != NULL; artifact = artifact->next) {
        complete = stage(scaffold, artifact);
    }
    for (ScaffoldArtifact *artifact = scaffold->first; complete && artifact != NULL; artifact = artifact->next) {
        complete = publish_one(scaffold, artifact);
    }
    cleanup(scaffold, !complete);
    scaffold->published = complete;
    return complete;
}

bool native_scaffold_publish(NativeScaffold *scaffold) {
    if (!scaffold->ready || scaffold->publish_attempted) {
        return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH, "A complete unpublished scaffold is required", NULL);
    }
    scaffold->publish_attempted = true;
    if (!directories(scaffold)) {
        return false;
    }
    const char *lock = scaffold_join(scaffold, scaffold->directory, "/", ".ama-native.lock");
    if (lock == NULL) {
        return false;
    }
    int descriptor = open(lock, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (descriptor < 0) {
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE,
            "Cannot acquire output lock; another generation or a retained lock requires attention", lock);
    }
    bool complete = false;
    if (close(descriptor) != 0) {
        scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot close output lock", lock);
    } else {
        complete = transaction(scaffold);
    }
    if (unlink(lock) != 0) {
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot release output lock", lock);
    }
    return complete;
}
