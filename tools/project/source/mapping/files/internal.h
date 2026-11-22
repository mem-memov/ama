/** @file
 * @brief Share exact captured-entry shape admission with closure-record adaptation.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires one source/destination shape contract for overlapping material maps.
 */
#ifndef AMA_SOURCE_FILE_MAP_INTERNAL_H
#define AMA_SOURCE_FILE_MAP_INTERNAL_H
#include "files.h"
/** Recognize exact prepared keys and present directory/file payload shapes, no I/O. */
bool source_file_map_entry_valid(const AcquisitionUnitEntry *, const char *destination);
#endif
