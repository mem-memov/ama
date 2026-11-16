/**
 * @file
 * @brief Declare model-level control descriptor composition, separate from byte writers.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) isolates these
 * model/identity-registry consumers from standalone IR queries and scalar writers.
 */
#ifndef AMA_CONTROL_DATA_H
#define AMA_CONTROL_DATA_H
#include "codegen/internal.h"

/** Emit a call descriptor using validated type/member identities and exact IR sites. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool codegen_control_invocation(NativeScaffold *, FILE *, const ScaffoldType *, const ControlNode *, const char *);
/** Emit fixed method input ordinals and exact typed preparation operation sites. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool codegen_control_list_inputs(NativeScaffold *, FILE *, const ScaffoldType *, const ControlNode *, const char *);
/** Emit the resolved assignment source and destination descriptors. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool codegen_control_assignment_data(NativeScaffold *, FILE *, const ControlNode *, const char *);
/** Emit the resolved output field/shared-constant descriptor. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool codegen_control_output_data(NativeScaffold *, FILE *, const SemanticSymbol *, const char *);
#endif
