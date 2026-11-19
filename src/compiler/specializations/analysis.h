/** @file
 * @brief Own the compiler's static equation, guarded registry and required-Type analysis stages.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) separates resolved Type inference from planning and execution.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) requires explicit reverse dependency ownership.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) shares cached Type identities without sharing runtime instance state.
 */
#ifndef AMA_COMPILER_SPECIALIZATION_ANALYSIS_H
#define AMA_COMPILER_SPECIALIZATION_ANALYSIS_H
#include "semantic/static/closure/plan.h"

/** Borrow one complete source inventory after native-definition admission.
 * All contained initializers are nonallocating; one initialization credit covers
 * their complete owner chain. No mutable runtime instance belongs in these caches.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) separates specialization identity from instance state.
 */
typedef struct {
    const StaticInventory *inventory;
    BindingPlan bindings;
    IdentityRegistry identities;
    StaticTermStore terms;
    StaticConstraintSolver constraints;
    StaticApplicationPlan applications;
    StaticGuardPlan guards;
    StaticRegistryPlan registry;
    StaticResultPlan results;
    StaticPublicationPlan publications;
    StaticConstantPlan constants;
    StaticConnectionPlan connections;
    StaticClosurePlan closure;
    Diagnostic error;
    bool attempted;
    bool valid;
} CompilerSpecialization;

/** Initialize healthy child owners without allocation; borrow the valid source
 * inventory and its longer-lived Project. Check once before destruction.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) separates initialization and fallible work.
 */
void compiler_specialization_init(CompilerSpecialization *, const StaticInventory *, SourceAllocator);
/** Destroy all partial children in reverse dependency order and zero the owner.
 * Borrowed inventory, source bytes and project ASTs remain alive and unchanged.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) bounds failed preparation.
 */
void compiler_specialization_destroy(CompilerSpecialization *);
/** Check definition-time lexical bindings throughout the loaded source closure,
 * register exactly the explicitly selected ordinary roots, then close their
 * required Type graph. Roots are count-sized borrowed original module pointers;
 * zero roots performs definition-only analysis, not implicit instantiation.
 * Native policy must already have passed. No ordinary capability/admission,
 * planning, C compilation or native execution occurs. Check once; failures retain
 * borrowed diagnostics and safely destructible partial children.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) checks definitions before specialization.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) requires explicit selected-root provenance.
 */
bool compiler_specialization_check(CompilerSpecialization *, const ProjectModule *const *, size_t);
#endif
