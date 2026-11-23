/** @file
 * @brief Share exact copy/delete/move intent categories without importing prepared plan ownership.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes retained originals, explicit deletion and copying followed by
 * source removal. Request recognition and checked operation intent use one enum.
 */
#ifndef AMA_SOURCE_OPERATION_KIND_H
#define AMA_SOURCE_OPERATION_KIND_H
/** A move never removes the originals of additional dependency copies. */
typedef enum { SOURCE_OPERATION_COPY, SOURCE_OPERATION_DELETE, SOURCE_OPERATION_MOVE } SourceOperationKind;
#endif
