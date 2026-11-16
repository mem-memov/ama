/**
 * @file
 * @brief Find one exact typed-IR operation without guessing a replacement selector.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) forbids fallback
 * matching. Queries use operation kind and pointer identities in the resolved IR.
 */
#ifndef AMA_CONTROL_SITE_RESOLUTION_H
#define AMA_CONTROL_SITE_RESOLUTION_H
#include "planning/plan.h"

/** A unique operation is required; zero/two matches are distinct invalid IR states. Component contract: [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
typedef enum { CONTROL_SITE_FOUND, CONTROL_SITE_MISSING, CONTROL_SITE_DUPLICATE } ControlSiteStatus;
/** Find nonliteral evaluation or the exact module/intrinsic literal staging kind. Component contract: [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
ControlSiteStatus codegen_control_binding_site(const ControlNode *, const SemanticStatement *,
    const SemanticBinding *, const ControlNode **);
/** Find declared parameter/result construction; no other field role has a site. Component contract: [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
ControlSiteStatus codegen_control_field_site(const ControlNode *, const SemanticStatement *,
    const SemanticSymbol *, const ControlNode **);
#endif
