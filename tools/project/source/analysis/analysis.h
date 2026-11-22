/** @file
 * @brief Own original checked source facts and exact independently cached test admission.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original graph facts;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) admits actual executable tests;
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * retains plain data without live compiler state;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed proofs.
 */
#ifndef AMA_PROJECT_SOURCE_ANALYSIS_H
#define AMA_PROJECT_SOURCE_ANALYSIS_H
#include "../compiler/client.h"
#include "../../acquisition/closure/plan/plan.h"
/** Original analysis preparation stages; compiler retains its exact typed child status. */
typedef enum { SOURCE_ANALYSIS_OK, SOURCE_ANALYSIS_INVALID, SOURCE_ANALYSIS_MEMORY,
    SOURCE_ANALYSIS_COMPILER } SourceAnalysisStatus;
/** One exact original candidate's retained proof/result. Invalid probes never
 * become Open through diagnostic guessing under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
typedef struct SourceAnalysisProbe {
    struct SourceAnalysisProbe *next;
    const DeliverySourceDefinition *definition;
    SourceCompilerClient compiler;
    AcquisitionClosureStatus status;
    SourceIssue issue;
} SourceAnalysisProbe;
/** Borrow immutable captured data/request storage through consumption; own copied
 * request/compiler names, original response and each retained probe. No source
 * discovery, staging-file mutation or runtime execution occurs under
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01).
 */
typedef struct {
    SourceArena arena;
    SourceCompilerClient original;
    DeliverySourceSnapshotView view;
    const char *compiler, *request_path;
    SourceAnalysisProbe *probes;
    const SourceAnalysisProbe *last_probe;
    SourceAnalysisStatus status;
    bool attempted, valid;
} SourceAnalysis;
/** Begin one original-analysis owner under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void source_analysis_init(SourceAnalysis *, SourceAllocator);
/** Retire probe clients before their original candidate owner and metadata under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void source_analysis_destroy(SourceAnalysis *);
/** Admit one already serialized immutable snapshot request through the compiler.
 * Copy compiler/request names and borrow view data until destruction. Publish
 * valid only after complete successful exact reply admission. Caller owns staging
 * file creation, exclusion and cleanup; this function never changes it. Repeated
 * calls refuse without replacing prior facts under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
SourceAnalysisStatus source_analysis_check(SourceAnalysis *, const DeliverySourceSnapshotView *,
    const char *compiler, const char *request_path);
/** T235 provider callback. Require an exact original ordinary/native candidate
 * handle; reject foreign/static definitions before spawning. Cache each candidate
 * once, retaining raw diagnostics and exact proof stage. Expected Open is healthy
 * support; all other failures remain failures. Require both captured byte equality
 * and original graph projection equality before accepting the proof. issue records
 * the requiring candidate witness, not a parsed/invented stderr location; the last
 * probe retains actual compiler diagnostics separately. Assign issue on every
 * completed callback. No execution under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
AcquisitionClosureStatus source_analysis_admit_test(void *, const DeliverySourceDefinition *, SourceIssue *);
#endif
