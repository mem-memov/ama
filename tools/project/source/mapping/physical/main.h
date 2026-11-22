/** @file
 * @brief Derive candidate ordinary main storage from a complete proposed final name index.
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05) uses plus
 * directories only for logical ancestors that are themselves modules;
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires added source/test paths to follow that topology, not mirrored plus guesses.
 */
#ifndef AMA_PROJECT_SOURCE_MAIN_PLACEMENT_H
#define AMA_PROJECT_SOURCE_MAIN_PLACEMENT_H
#include "source/memory/arena.h"
/** One proposed selected ordinary version root; keys are borrowed during a call.
 * physical is an already normalized absolute directory key. This record does not
 * fabricate a compiler ModuleVersion or claim final whole-version selection.
 */
typedef struct { const char *identity, *physical; } SourceVersionPlacement;
/** Invalid index and missing requested main are separate preparation failures. */
typedef enum { SOURCE_MAIN_PATH_OK, SOURCE_MAIN_PATH_INVALID,
    SOURCE_MAIN_PATH_MISSING, SOURCE_MAIN_PATH_MEMORY } SourceMainPathStatus;
/** Derive one main's owned absolute physical key from its canonical final name
 * and the complete proposed src/tests main-name set for the same version. Validate
 * all index names and uniqueness before use. Requested name must belong to that
 * set. Each logical ancestor that appears as a main uses a '+' directory; all
 * other components use ordinary directories. Tests have no automatic '+' merely
 * because they mirror source paths. No files are read or created. Output changes
 * only on success, scratch stays arena-owned. O(N squared + N*components) exact
 * path comparisons plus output bytes, O(output bytes) storage. The caller must
 * provide every proposed/surviving main in this version and separately confirm
 * the resulting candidates against common final discovery, collisions, actual
 * selected roots, context retention and publication safety.
 */
SourceMainPathStatus source_main_physical_path(SourceArena *, const SourceVersionPlacement *,
    const char *const *mains, size_t, const char *identity, const char **output);
#endif
