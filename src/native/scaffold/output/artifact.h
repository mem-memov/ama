/**
 * @file
 * @brief Represent complete output bytes and publication state without frontend data.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) lets filesystem
 * children depend on artifact records rather than the full compiler/scaffold.
 */
#ifndef AMA_SCAFFOLD_ARTIFACT_H
#define AMA_SCAFFOLD_ARTIFACT_H
#include "source/text/source.h"

/** Borrow destination paths/immutable bytes through publication; temporary paths
 * belong to the publication arena. Flags record partial preparation and the
 * successfully changed destination prefix.
 */
typedef struct ScaffoldArtifact {
    struct ScaffoldArtifact *next;
    const char *path;
    const SourceText *content;
    bool authored;
    bool executable;
    bool existed;
    bool published;
    const char *stage;
    const char *backup;
} ScaffoldArtifact;
#endif
