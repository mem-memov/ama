/**
 * @file
 * @brief Write control-operation C fragments from explicit names and selectors.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps writers
 * independent of frontend/model ownership. [STACK-01](../../../../spec/implementation/compiler.md#stack-01)
 * uses labels instead of C calls for Ama control transfer.
 * [FLOW-04](../../../../spec/language/control-flow.md#flow-04) routes a
 * converted header condition through its explicit release successors.
 */
#ifndef AMA_CONTROL_EMISSION_H
#define AMA_CONTROL_EMISSION_H
#include <stdio.h>
#include "source/text/source.h"

/** Emit label/source position, locate the current cell and optionally record the operation. */
bool codegen_control_location(FILE *, size_t selector, SourceSpan, bool record_operation);
/** Emit an explicit successor or terminal finish; no graph lookup is performed. */
bool codegen_control_jump(FILE *, bool finish, size_t selector);
/** Acquire a target; entry alone prepares its interface, refusal follows its selector. */
bool codegen_control_acquire(FILE *, const char *name, size_t alternative, bool entry);
/** Connect the resolved caller return and call-site selectors. */
bool codegen_control_connect(FILE *, size_t resume, size_t call_site);
/** Complete the current instance and follow its explicit runtime return link. */
bool codegen_control_complete(FILE *);
/** Release the current target, preserving technical-failure routing. */
bool codegen_control_release(FILE *);
/** Select a declared shared or instance result, then transfer it to the receiver. */
bool codegen_control_output(FILE *, const char *name, bool shared);
/** Emit the resolved receiver/source assignment operation. */
bool codegen_control_assignment(FILE *, const char *name);
/** Branch to the explicit true/false successors after condition conversion. */
bool codegen_control_condition(FILE *, const char *name, size_t yes, size_t no);
/** Convert the terminal result in its still-protected header owner, keeping
 * indices caller-local and routing success to explicit release-chain labels.
 */
bool codegen_control_condition_result(FILE *, const char *name, size_t yes, size_t no);
/** Reset the temporary status after the runtime has recorded a handled refusal. */
bool codegen_control_refusal(FILE *);
/** Enter the common terminal-failure path. */
bool codegen_control_failure(FILE *);
/** Store one already resolved continuation selector. */
bool codegen_control_save(FILE *, size_t resume);
/** Yield to the immediate caller or the explicit host boundary. */
bool codegen_control_yield(FILE *);
/** Consume the saved selector and use the validated resume dispatcher. */
bool codegen_control_resume(FILE *);
/** Clear the saved position on a fresh entry. */
bool codegen_control_restart(FILE *);
#endif
