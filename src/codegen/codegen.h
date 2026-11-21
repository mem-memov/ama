/**
 * @file
 * @brief Declare project artifact construction and verified native compilation.
 *
 * [COMP-04](../../spec/implementation/compiler.md#comp-04)
 * Borrow a valid nonauthoring frontend/options through emission; publication is explicit and uses the shared conflict-safe publisher.
 */
#ifndef AMA_PROJECT_CODEGEN_H
#define AMA_PROJECT_CODEGEN_H
#include "native/scaffold/scaffold.h"

/* Strict shared frontend and all borrowed source/option strings outlive emission.
 * Artifacts use the scaffold identity registry and conflict-safe publisher. */
/** Build shared declarations/header/body once without an application launcher.
 * Stateful library packaging retains this actual consumer under
 * [EMBED-01](../../spec/implementation/c-library.md#embed-01).
 * Failure leaves safely destructible partial artifacts and ready false.
 */
bool project_codegen_build(NativeScaffold *, ScaffoldOptions);
/** Admit explicit component/mode/root, retain exact launch-input capabilities and
 * stage application artifacts. Currently unsupported packaging refuses before
 * publication; later source-inclusion packaging is owned by T241 under
 * [EMBED-09](../../spec/implementation/c-library.md#embed-09).
 */
bool project_delivery_build(NativeScaffold *, ScaffoldOptions, const DeliveryRequest *);
/**
 * Require executable delivery, its explicit owned C entry artifact, ready unpublished
 * source artifacts and no previously retained executable. Clear ready
 * while staging/compile/verify/link/retain proceeds; set it only after success and best-effort
 * temporary cleanup.
 * Component contract: [COMP-04](../../spec/implementation/compiler.md#comp-04). */
bool project_codegen_compile(NativeScaffold *, FILE *errors);
#endif
