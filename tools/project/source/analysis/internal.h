/** @file
 * @brief Separate typed proof collection from candidate identity/cache admission.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) preserves exact root eligibility;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original graph facts across independent probes.
 */
#ifndef AMA_PROJECT_SOURCE_ANALYSIS_INTERNAL_H
#define AMA_PROJECT_SOURCE_ANALYSIS_INTERNAL_H
#include "analysis.h"
/** Complete one initialized candidate proof and its typed result; all raw data
 * remains probe-owned under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
void source_analysis_probe(SourceAnalysis *, SourceAnalysisProbe *);
#endif
