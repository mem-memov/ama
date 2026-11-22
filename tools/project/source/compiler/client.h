/** @file
 * @brief Own one shell-free captured-input compiler request and checked reply.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates project effects from compiler analysis;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * admits only exact prepared-input observations;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed prefixes.
 */
#ifndef AMA_PROJECT_SOURCE_COMPILER_CLIENT_H
#define AMA_PROJECT_SOURCE_COMPILER_CLIENT_H
#include "delivery/source/response/match.h"
#include "../../process/process.h"
/** Distinguish host/process refusal, unsuccessful child, capture I/O/allocation,
 * malformed reply and input mismatch. No failure grants graph authority under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
typedef enum { SOURCE_COMPILER_OK, SOURCE_COMPILER_INVALID, SOURCE_COMPILER_PROCESS,
    SOURCE_COMPILER_CHILD, SOURCE_COMPILER_CAPTURE, SOURCE_COMPILER_PROTOCOL,
    SOURCE_COMPILER_MISMATCH } SourceCompilerStatus;
/** Single-attempt owner. Raw streams survive as source objects separately from
 * response schema diagnostics; accepted graph spans belong to response.capture.
 * All borrowed fields expire at destruction under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    SourceManager raw;
    DeliverySourceResponse response;
    const SourceText *output, *diagnostics;
    ProjectProcessResult process;
    SourceCompilerStatus status;
    SourceStatus capture_status;
    bool attempted, valid;
} SourceCompilerClient;
/** Initialize empty result with allocator borrowed through destruction under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void source_compiler_client_init(SourceCompilerClient *, SourceAllocator);
/** Release result before raw diagnostic storage under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void source_compiler_client_destroy(SourceCompilerClient *);
/** Execute compiler --source-domain --snapshot request_path without a shell.
 * Caller owns the already serialized request and immutable input view throughout
 * this synchronous operation. Capture both streams, collect child normally, then
 * decode and compare only a successful exit. No source discovery or mutation.
 * Repeated calls refuse without replacing retained results. Raw diagnostics remain
 * available on child failure; host/stream failures can retain only partial data.
 * Exact input equality under [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * is required even for valid JSON and zero exit status.
 */
SourceCompilerStatus source_compiler_client_run(SourceCompilerClient *, const char *, const char *,
    const DeliverySourceSnapshotView *);
#endif
