/** @file
 * @brief Share bounded serialization operations for owned delivery artifacts.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) prevents partial
 * metadata publication; [NAME-02](../../../../spec/implementation/compiler.md#name-02)
 * requires complete identity keys rather than digest-only equality.
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01)
 * preserves concrete native origins and captured companion bytes.
 */
#ifndef AMA_CODEGEN_DELIVERY_METADATA_H
#define AMA_CODEGEN_DELIVERY_METADATA_H
#include "codegen/internal.h"
#include "tools/output/json/json.h"
/** Serialize arbitrary bytes as lowercase hex with an exact decimal-string length.
 * The borrowed writer retains output failure under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool delivery_metadata_bytes(ToolJsonWriter *, const unsigned char *, size_t);
/** Serialize the finite registered Type graph with component-local references;
 * cycles never recursively expand under
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02).
 */
bool delivery_metadata_types(ToolJsonWriter *, const NativeScaffold *);
/** Serialize actual preceding output records and the explicitly selected C unit.
 * The delivery manifest is staged last and does not recursively list itself.
 * [STACK-04](../../../../spec/implementation/compiler.md#stack-04) verifies
 * selected entry units rather than inferring them from filenames.
 */
bool delivery_metadata_outputs(ToolJsonWriter *, const NativeScaffold *);
/** Emit ordered actual native origins with captured source/instruction bytes
 * under [BUILD-01](../../../../spec/implementation/application-builds.md#build-01).
 */
bool delivery_metadata_native(ToolJsonWriter *, const NativeScaffold *);
#endif
