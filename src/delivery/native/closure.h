/** @file
 * @brief Select component-local native origins from concrete executable plans.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01) excludes
 * unused source definitions and applies each native origin once per component;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) keeps borrowed plan
 * and source lifetimes explicit through independently owned relation records.
 */
#ifndef AMA_DELIVERY_NATIVE_CLOSURE_H
#define AMA_DELIVERY_NATIVE_CLOSURE_H
#include "compiler/frontend.h"

/** Arena-owned ordered origin, borrowing the checked source catalog and concrete
 * plans. A physical source belongs to one resolved native module under
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01).
 */
typedef struct DeliveryNativeOrigin {
    struct DeliveryNativeOrigin *next;
    const ModuleSource *source;
} DeliveryNativeOrigin;

/** Prepare a strictly logical-name-ordered native origin table from a valid
 * executable frontend, without filesystem access or CMake execution. Deduplicate
 * exact module identity; reject inconsistent source owners for the same name.
 * Output changes only on success; failed arena scratch remains caller-owned.
 * Borrowed frontend/catalog must outlive the result. Time O(P*N), space O(N),
 * for P planned types and N distinct native origins. Empty closure is valid.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01)
 * supplies the participating-module domain and component-local deduplication.
 */
bool delivery_native_origins(SourceArena *, const CompilerFrontend *,
    const DeliveryNativeOrigin **, Diagnostic *);
#endif
