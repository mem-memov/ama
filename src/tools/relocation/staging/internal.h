/** @file
 * @brief Separate original-span rewriting, final registry construction and graph comparison.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * permits publication only after every independent preparation phase succeeds.
 */
#ifndef AMA_RELOCATION_STAGING_INTERNAL_H
#define AMA_RELOCATION_STAGING_INTERNAL_H
#include "staging.h"
/** Preserve the first allocation-free diagnostic and safely retained partial output. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_fail(RelocationStaging *, RelocationStatus, const char *, SourceSpan);
/** Create all final source records without interpreting draft or opaque bytes. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_texts(RelocationStaging *);
/** Copy exact sorted nonoverlapping replacements bound to the supplied original bytes into final-path text. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_rewrite(RelocationStaging *, const SourceText *, const char *,
    const RelocationPathEdit *const *, size_t, const SourceText **);
/** Map an original boundary through the sorted nonempty edit sequence accepted by rewrite, with its proven representable final length. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_offset(const SourceText *, const RelocationPathEdit *const *, size_t, size_t, size_t *);
/** Capture all carried entries and original source/native dependencies without interpretation. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_files(RelocationStaging *);
/** Capture or reuse one exact physical entry, checking optional expected identity metadata. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
RelocationStagedFile *relocation_staging_capture(RelocationStaging *, const char *, const char *, const struct stat *);
/** Compare metadata fields needed to detect a changed captured input, without struct padding. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_metadata(const struct stat *, const struct stat *);
/** Read literal symlink bytes without following its target, growing checked storage until complete. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_link(RelocationStaging *, const char *, const SourceText **);
/** Build the independently parsed and typed final catalog from prepared bytes. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_domain(RelocationStaging *);
/** Check every final dependency against its original edge under the complete identity map. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_graph(RelocationStaging *);
/** Validate and prepare only explicitly supplied documented settings and output invalidations. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_settings(RelocationStaging *, const RelocationStagingRequest *);
/** Normalize one explicit file/output through a verified parent without accepting spelling aliases. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
const char *relocation_staging_setting_path(RelocationStaging *, const char *, bool, bool *);
/** Parse, map exact module-field spans and independently validate one library configuration. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_configuration(RelocationStaging *, const char *);
/** Check explicit generated output scope against protected sources, roots and settings. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_invalidation(RelocationStaging *, const char *);
/** Check actual parent mutation authority for required writes outside the carried tree. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_writes(RelocationStaging *);
/** Install captured native companions under their projected paths without filesystem reads. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_native(RelocationStaging *, const RelocationStagedSource *, const ModuleSource *);
#endif
