/** @file
 * @brief Share explicit dependency-copy policy between source requests and closure preparation.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * keeps cloning and concrete configuration extension mutually exclusive and
 * refuses ordinary cross-version copies without one of those explicit policies.
 */
#ifndef AMA_SOURCE_COPY_DEPENDENCY_MODE_H
#define AMA_SOURCE_COPY_DEPENDENCY_MODE_H
/** KEEP/EXTEND select the same unit closure; permission extension is a later
 * checked configuration operation. No mode authorizes acquisition or execution. */
typedef enum { SOURCE_COPY_KEEP_EXTERNAL, SOURCE_COPY_EXTEND_PACKAGE,
    SOURCE_COPY_CLONE_DEPENDENCIES } SourceCopyDependencyMode;
#endif
