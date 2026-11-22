/** @file
 * @brief Admit typed entry results only against unchanged original bytes and graph metadata.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original resolved graph facts;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) distinguishes exact Open from every failed proof.
 */
#include "internal.h"
#include "../graph/equal.h"
/** Preserve explicit allocation failure where the data/process client knows it;
 * all other failed stages remain preparation errors under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
static AcquisitionClosureStatus failed(const SourceCompilerClient *client) {
    bool memory = client->status == SOURCE_COMPILER_CAPTURE
        && (client->capture_status == SOURCE_OUT_OF_MEMORY || client->capture_status == SOURCE_LIMIT);
    memory = memory || (client->status == SOURCE_COMPILER_PROTOCOL && client->response.status == SOURCE_JSON_OUT_OF_MEMORY);
    return memory ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_PREPARATION_FAILED;
}
void source_analysis_probe(SourceAnalysis *analysis, SourceAnalysisProbe *probe) {
    SourceCompilerStatus status = source_compiler_client_run(&probe->compiler, analysis->compiler,
        analysis->request_path, &analysis->view, probe->definition->identity);
    if (status != SOURCE_COMPILER_OK) {
        probe->issue.message = "Independent compiler entry proof failed";
        probe->status = failed(&probe->compiler); return;
    }
    if (!source_graph_equal(analysis->original.response.definitions, probe->compiler.response.definitions)) {
        probe->issue.message = "Independent entry proof changed the original checked source graph";
        probe->status = ACQUISITION_CLOSURE_PREPARATION_FAILED; return;
    }
    if (probe->compiler.probe_state == DELIVERY_PROBE_EXECUTABLE) { probe->status = ACQUISITION_CLOSURE_OK; }
    else if (probe->compiler.probe_state == DELIVERY_PROBE_OPEN) { probe->status = ACQUISITION_CLOSURE_OPEN_TEST; }
    else {
        probe->issue.message = "Independent compiler response contains no entry proof";
        probe->status = ACQUISITION_CLOSURE_PREPARATION_FAILED; return;
    }
    probe->issue = (SourceIssue){0};
}
