/** @file
 * @brief Construct closed prepared-text and supplemental-write sets from typed operation producers.
 * [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires an explicit owner for every source mutation; [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * grants no authority to arbitrary successfully applied configuration edits.
 */
#ifndef AMA_SOURCE_OPERATION_WRITE_JOIN_H
#define AMA_SOURCE_OPERATION_WRITE_JOIN_H
#include "../configuration/configuration.h"
#include "../module/module.h"
#include "../../../removal/selection/material.h"
#include "../../../graph/sites.h"
/** Borrow genuine immutable input owners through all joined-plan consumers.
 * main_texts contains only actual C-main preparations. All other rewrites come
 * from recognized typed writers. No raw supplemental change array is accepted. */
typedef struct {
    const SourceCopyClosure *copy;
    const SourceObservedImage *observed;
    const SourceCloneMap *mains;
    const SourceFileMap *files;
    const SourceGraphPreparedText *main_texts;
    const SourceRemovalSelection *removal;
    const SourceVersionInitialization *const *initializations;
    size_t initialization_count;
    const SourceOperationConfigurationWrite *const *configurations;
    size_t configuration_count;
    const SourceOperationModuleWrite *const *modules;
    size_t module_count;
} SourceOperationWritesInputs;
/** Prepared copied text is distinguished from source-removal/initialization and
 * existing configuration changes; metadata never grants a second write route. */
typedef enum { SOURCE_OPERATION_PREPARED_MAIN, SOURCE_OPERATION_PREPARED_MODULE,
    SOURCE_OPERATION_PREPARED_CONFIGURATION } SourceOperationPreparedKind;
/** Own linked query rows while borrowing the applied producer's edit/results. */
typedef struct SourceOperationPreparedWrite {
    struct SourceOperationPreparedWrite *next;
    SourceGraphPreparedText text;
    SourceOperationPreparedKind kind;
    union {
        const SourceGraphPreparedText *main;
        const SourceOperationModuleWrite *module;
        const SourceOperationConfigurationWrite *configuration;
    } owner;
} SourceOperationPreparedWrite;
/** Every extra change has exactly one typed source of authority. */
typedef enum { SOURCE_OPERATION_EXTRA_REMOVAL, SOURCE_OPERATION_EXTRA_INITIALIZATION,
    SOURCE_OPERATION_EXTRA_CONFIGURATION } SourceOperationExtraKind;
/** Index-aligned with changes; initialized configuration replacement retains its
 * initializer through the configuration writer's exact borrowed inputs. */
typedef struct {
    SourceOperationExtraKind kind;
    union {
        const SourceRemovalSelection *removal;
        const SourceVersionInitialization *initialization;
        const SourceOperationConfigurationWrite *configuration;
    } owner;
} SourceOperationExtraOwner;
/** Preserve admission, duplicate ownership, wrong field scope and allocation errors. */
typedef enum { SOURCE_OPERATION_WRITES_OK, SOURCE_OPERATION_WRITES_INVALID,
    SOURCE_OPERATION_WRITES_SCOPE, SOURCE_OPERATION_WRITES_DUPLICATE,
    SOURCE_OPERATION_WRITES_MATERIAL, SOURCE_OPERATION_WRITES_LIMIT,
    SOURCE_OPERATION_WRITES_MEMORY } SourceOperationWritesStatus;
/** Own copied list nodes, extra metadata and normalized symbolic classes. Borrow
 * immutable producer bytes and original facts until every plan consumer retires.
 * valid certifies the closed write set, not root/client intent or final semantics. */
typedef struct {
    SourceArena arena;
    SourceOperationWritesInputs inputs;
    SourceOperationPreparedWrite *first, *last;
    const SourceGraphPreparedText *prepared;
    SourceImageChange *changes;
    SourceOperationExtraOwner *owners;
    size_t count, capacity;
    SourceRemovalMaterial removal_material;
    const char *failed_path;
    SourceOperationWritesStatus status;
    bool attempted, valid;
} SourceOperationWrites;
/** Begin a finite owned join without effects. */
void source_operation_writes_init(SourceOperationWrites *, SourceAllocator);
/** Retire row/metadata scratch before borrowed writer/result owners expire. */
void source_operation_writes_destroy(SourceOperationWrites *);
/** Join once. Derive actual D removals; admit only necessary typed initialization
 * rows and recognized regular configuration updates. Carried package/module INI
 * writes join P text rather than becoming extra mutations. Source preparations
 * must name actual C mains; duplicates across all routes refuse. Preserve byte-
 * identical existing configurations without a needless physical rewrite. Assign
 * new metadata classes through the shared exact symbolic allocator.
 * No I/O, execution, source mutation or untyped fallback. Failure prefixes remain
 * owned/invalid. With R removals, I initializations, G package writers, M module
 * writers and T main preparations, extra count is at most R+3I+G and prepared
 * count at most T+M+G. Checked arrays/list nodes own this bounded storage; no
 * preparation dynamically expands its inputs. Pairwise ownership/key scans are
 * quadratic in these counts, plus original image/P/clone lookups and shared
 * placement/symbolic-class bounds. The least-unused class exists within one more
 * than the finite occupied class count, or representation exhaustion refuses. Full material, original cut,
 * root/client, compiler/site and transaction authorization remain mandatory.
 */
SourceOperationWritesStatus source_operation_writes_build(SourceOperationWrites *, const SourceOperationWritesInputs *);
#endif
