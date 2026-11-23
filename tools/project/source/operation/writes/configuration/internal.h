/** @file
 * @brief Separate configuration recipe provenance from its shared selector/alias operations.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * grants write authority only to the actual original/carried/initialized scope.
 */
#ifndef AMA_SOURCE_OPERATION_CONFIGURATION_WRITE_INTERNAL_H
#define AMA_SOURCE_OPERATION_CONFIGURATION_WRITE_INTERNAL_H
#include "configuration.h"
/** Check exact owner membership, actual destination material and recipe category. */
SourceOperationConfigurationWriteStatus source_operation_configuration_write_admit(const SourceOperationConfigurationWriteInputs *);
/** Prove one carried package document against raw selection and exact P bytes. */
bool source_operation_configuration_carried(const SourceOperationConfigurationWriteInputs *,
    const SourcePackageScope *, const SourcePackageScope *);
/** Every supplied version pair must describe an actual carried raw configuration. */
bool source_operation_configuration_versions(const SourceOperationConfigurationWriteInputs *);
#endif
