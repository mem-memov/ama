/** @file
 * @brief Share narrow parse, resolution and dependency contracts within library building.
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) separates records,
 * nominal/schema validation, naming and input capture without duplicating semantics.
 */
#ifndef AMA_LIBRARY_INTERNAL_H
#define AMA_LIBRARY_INTERNAL_H
#include "model.h"
/** Record an allocation-free source-backed error and leave the build unusable. */
bool library_fail(LibraryBuild *, SourceSpan, const char *);
/** Allocate zero-initialized model storage or retain a memory diagnostic. */
void *library_allocate(LibraryBuild *, size_t);
/** Decode one exact record with bounded, nonempty fields into owned model declarations. */
bool library_configuration_record(LibraryBuild *, SourceSpan);
/** Require a portable nonreserved ASCII public identifier, independent of Ama naming. */
bool library_public_identifier(const char *);
/** Find an explicitly declared schema by exact public identifier, or return null. */
LibrarySchema *library_schema_find(const LibraryBuild *, const char *);
/** Find an explicitly declared export by exact public identifier, or return null. */
LibraryExport *library_export_find(const LibraryBuild *, const char *);
/** Resolve executable export identities after strict frontend completion. */
bool library_resolve_exports(LibraryBuild *);
/** Resolve explicit scalar/list wire schemas and their exact supported nominal contracts. */
bool library_resolve_schemas(LibraryBuild *);
/** Resolve every mapping and prove complete parameter/result coverage without implicit schemas. */
bool library_resolve_bindings(LibraryBuild *);
/** Build a complete framed ABI key from explicit records, excluding comments and physical paths. */
bool library_identity_parts(LibraryBuild *);
/** Register component-qualified names with full-key collision checking. */
bool library_resolve_identities(LibraryBuild *);
/** Capture a library consumer, raw configuration and explicit additional link files. */
bool library_capture_inputs(LibraryBuild *);
#endif
