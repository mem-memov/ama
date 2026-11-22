/** @file
 * @brief Share typed original input-use construction without file or source-text discovery.
 * [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * retains surviving configuration/native/declared inputs;
 * [PROJECT-01](../../../../../../spec/implementation/project-tools.md#project-01)
 * keeps compiler state and execution outside project-side review.
 */
#ifndef AMA_PROJECT_SOURCE_REMOVAL_INPUTS_INTERNAL_H
#define AMA_PROJECT_SOURCE_REMOVAL_INPUTS_INTERNAL_H
#include "../internal.h"
/** Private append-only list; publish only after all original owners succeed. */
typedef struct {
    SourceRemovalSelection *selection;
    SourceRemovalInputUse *first, **tail;
} SourceRemovalInputBuilder;
/** Append one exact captured requirement; span zero witnesses the input file,
 * not a guessed source-language occurrence. Names/bytes retain original owners.
 */
SourceRemovalSelectionStatus source_removal_input_append(SourceRemovalInputBuilder *,
    const DeliverySourceDefinition *, const char *, const SourceText *);
/** Retain the original module/package configuration authority. */
SourceRemovalSelectionStatus source_removal_input_configuration(SourceRemovalInputBuilder *,
    const DeliverySourceDefinition *, const SourceCapturedUnit *, const DeliveryResponseSource *);
/** Retain captured native body/header/ABI and optional authored CMake companion. */
SourceRemovalSelectionStatus source_removal_input_native(SourceRemovalInputBuilder *,
    const DeliverySourceDefinition *, const SourceCapturedUnit *, const DeliveryResponseSource *);
#endif
