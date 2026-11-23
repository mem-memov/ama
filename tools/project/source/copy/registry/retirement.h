/** @file
 * @brief Distinguish selected-definition retirement from authorized physical removal.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * permits a selected external definition to become ignored during promotion
 * without deleting its files. [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * still checks the independently supplied actual physical removal cut.
 */
#ifndef AMA_SOURCE_COPY_REGISTRY_RETIREMENT_H
#define AMA_SOURCE_COPY_REGISTRY_RETIREMENT_H
#include "../../capture/image.h"
/** Own only the pointer array in the caller's arena; names borrow original units. */
typedef struct { const char *const *names; size_t count; } SourceRegistryRetirement;
/** Separate malformed input from a scratch allocation refusal. */
typedef enum { SOURCE_REGISTRY_OK, SOURCE_REGISTRY_INVALID, SOURCE_REGISTRY_MEMORY } SourceRegistryStatus;
/** Derive exact original names physically removed or no longer selected at their
 * original main path. Every actual removal name must belong to the original
 * selected capture; duplicates are harmless. New names are not retirements.
 * A changed path can preserve nominal identity, and never authorizes erasure.
 * Checked captures provide unique identities. No I/O, guessing, source changes
 * or semantic admission. Failure leaves output untouched and scratch arena-owned.
 * Finite O(U*(F+R)) exact name/path comparisons and O(U) pointer storage, for
 * U original/F final units and R physical removal names. Borrow both captures
 * until use completes; borrow original names through the retirement's lifetime.
 */
SourceRegistryStatus source_copy_registry_retirement(SourceArena *, const SourceRootCapture *,
    const SourceImageCapture *, const char *const *, size_t, SourceRegistryRetirement *);
#endif
