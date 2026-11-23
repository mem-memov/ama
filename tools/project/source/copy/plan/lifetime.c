/** @file
 * @brief Own final plan evidence in reverse dependency order on every failed prefix.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps child catalog/environment borrowers alive until their dependants retire.
 */
#include <string.h>
#include "plan.h"
void source_copy_plan_init(SourceCopyPlan *plan, SourceAllocator allocator) {
    memset(plan, 0, sizeof(*plan)); source_arena_init(&plan->arena, allocator);
    source_image_capture_init(&plan->capture, allocator);
    source_package_environment_init(&plan->environment, allocator);
}
void source_copy_plan_destroy(SourceCopyPlan *plan) {
    source_package_environment_destroy(&plan->environment); source_image_capture_destroy(&plan->capture);
    source_arena_destroy(&plan->arena); memset(plan, 0, sizeof(*plan));
}
