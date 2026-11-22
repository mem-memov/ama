/** @file
 * @brief Own membership metadata independently of borrowed source snapshots.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) gives failed
 * preparation one destructor; [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03)
 * retains original source provenance until reports are consumed.
 */
#include "members.h"
void acquisition_members_init(AcquisitionMembers *members, SourceAllocator allocator) {
    *members = (AcquisitionMembers){0}; source_arena_init(&members->arena, allocator);
}
void acquisition_members_destroy(AcquisitionMembers *members) {
    source_arena_destroy(&members->arena); *members = (AcquisitionMembers){0};
}
