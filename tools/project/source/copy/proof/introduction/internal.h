/** @file
 * @brief Separate original supplier provenance from graph conservation after version reselection.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires both immutable material and original target preservation.
 */
#ifndef AMA_SOURCE_COPY_INTRODUCTION_INTERNAL_H
#define AMA_SOURCE_COPY_INTRODUCTION_INTERNAL_H
#include "introduction.h"
/** Check original ordinary coverage and unchanged main/companion/configuration material. */
SourceIntroductionStatus source_copy_introduction_origin(SourceArena *, const SourceObservedImage *,
    const SourceCapturedUnit *, SourceIntroductionIssue *);
/** Borrow exactly the original mapping or selected source for finite membership queries. */
const SourceCloneMapping *source_copy_introduction_clone(const SourceCloneMap *, const char *, bool);
/** Restrict final authors and reject a newly selected supplier replacing an old target. */
SourceIntroductionStatus source_copy_introduction_graph(SourceArena *, const SourceCopyIntroduction *,
    const SourceCloneMap *, const DeliverySourceDefinition *, const DeliverySourceDefinition *,
    const DeliverySourceDefinition **, SourceIntroductionIssue *);
#endif
