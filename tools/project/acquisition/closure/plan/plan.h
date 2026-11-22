/** @file
 * @brief Close finite prepared source/test facts without compiler state or transport effects.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) supplies structural
 * units; [PKG-08](../../../../../spec/project/packages.md#pkg-08) supplies test
 * association and finite least closure; [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01)
 * keeps provider preparation separate from this project-side controller.
 * Prepared facts and exact joins replace heuristic source discovery.
 * [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05)
 * The enclosing operation owns partial preparation and borrowed lifetime boundaries.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 */
#ifndef AMA_ACQUISITION_CLOSURE_PLAN_H
#define AMA_ACQUISITION_CLOSURE_PLAN_H
#include "../members/members.h"
#include "../tests/association.h"
#include "../tests/roots.h"
#include "../dependencies/expand.h"
/** Captured topology-only ancestor. It retains its main and companions, with no
 * sibling/subtree activation under [PKG-07](../../../../../spec/project/packages.md#pkg-07).
 */
typedef struct AcquisitionContextFact {
    struct AcquisitionContextFact *next;
    const char *identity;
    const AcquisitionUnitEntry *files;
} AcquisitionContextFact;
/** Separately identified required source/native/configuration/test input group.
 * These are exact captured declarations or explicit caller-selected test data,
 * never guessed assets or C/CMake text scans under
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07). Each group retains
 * its own bound version source, or NULL for explicitly unversioned input bytes.
 */
typedef struct AcquisitionDeclaredInputs {
    struct AcquisitionDeclaredInputs *next;
    const AcquisitionVersionSource *source;
    const AcquisitionUnitEntry *files;
} AcquisitionDeclaredInputs;
/** One complete prepared expansion. Providers use the authoritative source
 * catalog, complete structural captures and the checked compiler export. Every
 * module in the active owned subtree appears in submodules; active definitions
 * in associated tests appear in definitions. Draft test material may remain only
 * in test_files. Configuration is required for ordinary versions; guidance is
 * optional but a supplied unreadable/non-file entry is a provider failure, never
 * a missing entry. Providers and all borrowed source owners outlive the plan.
 * For an explicit version request module is NULL, files retain its source tree
 * (including an empty root), submodules enumerate its src definitions, and the
 * same bound source/configuration/guidance facts apply. No pseudo-module is created.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) and
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) define these exact facts;
 * this interface does not authorize guessed assets or skipped compiler failures.
 */
typedef struct {
    const ModuleSource *module;
    const AcquisitionVersionSource *source;
    const AcquisitionUnitEntry *files, *configuration, *guidance;
    const AcquisitionContextFact *contexts;
    const char *const *submodules;
    size_t submodule_count;
    const AcquisitionTestDirectory *test_directory;
    const AcquisitionUnitEntry *test_files;
    const DeliverySourceDefinition *definitions;
    const AcquisitionDeclaredInputs *declared_inputs;
} AcquisitionExpansion;
/** Non-OK states stop publication; OPEN_TEST is a healthy admission-only result. */
typedef enum { ACQUISITION_CLOSURE_OK, ACQUISITION_CLOSURE_INVALID,
    ACQUISITION_CLOSURE_MISSING_TEST, ACQUISITION_CLOSURE_CONFLICT,
    ACQUISITION_CLOSURE_OUT_OF_MEMORY, ACQUISITION_CLOSURE_PREPARATION_FAILED, ACQUISITION_CLOSURE_OPEN_TEST } AcquisitionClosureStatus;
/** Prepare one exact active unit from stable already selected snapshots. The
 * provider owns capture/compiler-adapter details and returns a complete fact only
 * after ordinary definition checks. It must not execute native/CMake code, run
 * tests, fetch sources or mutate inputs. Refusal retains its original explanation.
 * [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01)
 * assigns acquisition/transport effects to their separate explicit operation.
 */
typedef AcquisitionClosureStatus (*AcquisitionExpansionProvider)(void *, const char *, AcquisitionExpansion *, SourceIssue *);
/** Classify one direct ordinary/native candidate through normal independent entry
 * analysis without execution. OK means an executable proof; OPEN_TEST means an
 * exact pending generic root key at the inference fixed point. Every actual
 * compilation error remains fatal. Original formal count alone cannot reject a
 * candidate because internal constraints can close those formals. Providers
 * retain diagnostic source ownership through plan destruction under
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08).
 */
typedef AcquisitionClosureStatus (*AcquisitionTestAdmission)(void *, const DeliverySourceDefinition *, SourceIssue *);
/** Plain provider contract shared for one immutable operation. No callback may
 * retain mutable compiler state in this owner or implement heuristic fallback
 * under [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05).
 */
typedef struct { void *context; AcquisitionExpansionProvider expand; AcquisitionTestAdmission admit_test; } AcquisitionClosureProvider;
/** Own the role/member fixed point and selected independent tests. Provider facts,
 * source/version owners and original bytes remain borrowed. valid alone permits
 * output consumption. Failed scratch/partial sets remain owned for teardown under
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    SourceArena arena;
    AcquisitionRoles roles;
    AcquisitionMembers members;
    AcquisitionTestRoot *tests;
    AcquisitionClosureStatus status;
    SourceIssue issue;
    bool attempted, valid;
} AcquisitionClosure;
/** Initialize a nonallocating owner before its one closure computation. */
void acquisition_closure_init(AcquisitionClosure *, SourceAllocator);
/** Release complete/failed metadata before borrowed provider captures and spans. */
void acquisition_closure_destroy(AcquisitionClosure *);
/** Select complete canonical module identities, then expand each active identity
 * once until no new source/test/submodule identity appears. Explicit version seeds
 * expand their complete source tree and enumerate src module seeds through the
 * provider, retaining version/configuration evidence even for an empty tree. Arrays are
 * borrowed only during this call; providers outlive retained facts. Duplicate
 * seeds/edges are idempotent. Refusal leaves valid false; never publish the partial
 * member set. On success the set is the least one closed under all supplied exact
 * structural, test and compiler-edge facts. Termination assumes a finite reachable
 * snapshot graph and finishing providers, as [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * requires; no arbitrary provider is promised to terminate.
 */
AcquisitionClosureStatus acquisition_closure_compute(AcquisitionClosure *, const char *const *, size_t,
    const AcquisitionClosureProvider *);
#endif
