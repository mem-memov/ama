# Applying a prepared relocation

T113's `src/tools/relocation/transaction/` consumes a ready
[staging plan](relocation-staging.md) and applies its filesystem effects together:
the selected tree, outside importers, recognized library settings and explicit
generated-output invalidations. It preserves originals until the commit decision
and restores them after ordinary precommit failure, under
[TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04).
It never runs Ama, native constructors, embedded flags or a linker.

This component implements synchronous application and in-process inverse cleanup.
T114 adds [persistent preparation, independent recovery and owned reports](relocation-recovery.md),
including terminal outcome receipts. The command/client boundary remains T115.

## Authority and ownership

Initialize `RelocationTransaction`, apply once, inspect and destroy. The ready
`RelocationStaging`, its references and preview survive the transaction. Request
strings and the optional synchronous fault callback survive apply; normalized
recovery paths and all application records are transaction-owned. No callback is
used during destruction. Destruction closes outstanding leases and releases memory;
it never silently retries an incomplete inverse or deletes retained disk data.

The caller supplies an existing recovery directory outside the moved tree and
outside invalidated output subtrees. Descriptor-backed exclusive leases cover
original/proposed source roots and their parents, the source directory, endpoint
parents, every contributing input parent, existing output directories and recovery
storage. A future root below D is locked through its actual captured counterpart
below S. Shared physical directory identities reuse one descriptor. Nonblocking
`flock` rejects contention rather than waiting in a cycle; held prefixes are released
on every terminal path. Locks survive pathname renames because they refer to directory
objects, and no lock files are inserted into the source tree.

The supported implementation uses Linux directory leases and `renameat2` with
`RENAME_NOREPLACE`. Actual local workspace and `/tmp` filesystem combinations are
tested. Unsupported OS/filesystem operations refuse rather than silently weakening
no-overwrite behavior. Leases coordinate cooperating writers; other tools/clients
must honor the same exclusive project operation. They cannot prevent an arbitrary
noncooperating process from writing through an already open descriptor. That is an
explicit environmental precondition, not an inferred absence of competing writers.

## Revalidate the complete scope

Reconstruct the original explicit roots, optional destination root, original entry,
library files and output inventory from retained canonical records. Under the leases,
run discovery, original binding analysis and final staging again. Compare all source
candidates, original/final physical and logical identities, active flags, exact
original/prepared bytes, protected input roles and output existence/metadata. Compare
complete sets, not only previously listed selected files. This detects an added
unused module outside S and newly present native companions as well as changed or
removed original sources. Temporary compiler owners expire before application
continues; retained diagnostics never borrow their destroyed storage.

Run this check once before preparing replacement contents and again afterwards.
Every original regular replacement is also checked immediately before backup. All
checks use actual bytes and metadata, not hashes, guessed timestamps or a textual
namespace prefix replacement. Temporary private directory names do not become Ama
source namespaces under the ordinary discovery rules.

## Prepare inverses before publication

Each action owns an exclusively created sibling slot on its target filesystem.
The slot has `prepared` and `original` child locations. Ownership is linked before creation. T114 records each later slot name before
`mkdir` and its physical identity before copying data, so interruption and allocation
failure cannot lose a nonempty private copy. The recovery slot is created first;
its complete intent scope precedes other slots.

The destination is an independent copy of the complete selected physical inventory.
Directory parents precede children, following T110's breadth-first inventory.
Ama files receive exact prepared bytes; native files and other regular assets retain
their binary bytes; symbolic links retain literal target bytes without traversal.
Explicitly invalidated generated descendants are omitted. Finalize directory metadata
in reverse order after every descendant has been created. Regular data is read back
with bounded stack buffers and compared byte-for-byte, then owner, mode and timestamps
are applied and synchronized. Ownership precedes mode because chown may clear special
permission bits. Links use no-follow timestamp/ownership operations; unsupported link
permission models reject without following their targets.

External files whose bytes are unchanged need no action. Changed importers and
library configurations receive separately prepared sibling files. Existing generated
outputs outside S receive removal actions; their originals are saved by rename, not
rewritten or interpreted. Generated entries inside S remain in the original backup
until commit and do not appear in the destination copy.

The implementation copies the destination tree on both same and different
filesystems. Consequently final physical inode identities can change; clients must
use the returned path/settings map and refresh their source views. Original objects
remain intact in their own filesystem's backup slots until commit. The approach
avoids assuming a cross-device rename or assuming that one rename publishes a
multi-file update.

## Forward and inverse laws

Let P be an action's public path, B its private original location and T its private
prepared location. The supported transitions are:

| Action | Before | Forward transition | Inverse |
|---|---|---|---|
| Remove | O at P; B absent | P -> B | B -> P |
| Create | N at T; P absent | T -> P | P -> T |
| Replace | O at P; N at T; B absent | P -> B, then T -> P | P -> T, then B -> P |

The complete action order first saves S, publishes D, replaces outside files and
retires outside outputs. T112's protected-input and overlap checks make public action
paths disjoint: D was absent; outside inputs cannot lie in S or D; output removals
cannot consume a protected input or another output. Private slots are independently
owned, and preparation is revalidated before mutation. Therefore inverse actions in
reverse order restore the composite original state.

Intent flags precede rename calls. Inverse operations probe actual absent/expected/
foreign identities rather than treating a failed acknowledgement as proof that no
rename happened. Only owned prepared objects are withdrawn. An original already
restored is a no-op; an unexpected existing object is preserved. No-replace rename
prevents a race from overwriting a new destination. Independent inverse actions
continue after another inverse fails. Neither inverse execution nor a retry applies
an Ama text edit again.

Before commit, O objects are never destroyed or changed in place. Successful inverse
restores their original paths, bytes, modes, owners and physical identities. Filesystem
access/change timestamps may reflect normal reading/renaming; they are not source
bytes and cannot be universally rolled back by POSIX operations.

## Journal decision and failure outcomes

A framed prepared inventory records action kinds, exact path lengths/bytes and
original/prepared object identities before publication. Path contents cannot create
new records by containing newlines. Prepared files, slot directories and their
parent directory entries are synchronized before applying actions.

After all forward actions, synchronize affected slot/parent entries and append the
commit record. A failed provisional acknowledgement does not immediately permit an
inverse: truncate back to the complete prepared inventory and synchronize that
retraction first. If retraction cannot be established, set `decision_pending`, retain
all copies and report an incomplete outcome. Never restore some original paths while
a potentially durable commit decision still directs recovery to complete cleanup.

Once the commit decision succeeds, `committed` is true and rollback is prohibited.
Old backup slots can then be retired. The committed journal remains as an outcome
receipt after backup cleanup; deleting the only result record before acknowledging
completion would lose recovery/reporting authority. T114 supplies receipt replay and
final outcome reporting.

Before commit, an ordinary refusal restores attempted actions and removes private
copies. A failed inverse retains its originals, slots and journal. After commit, a
cleanup refusal retains the new project and the committed journal. Both cases return
failure with `INCOMPLETE` state; the primary cause and first secondary cleanup cause
remain separate. Successfully restored failure returns `ROLLED_BACK`, not success.
Destroying any of these owners is not permission to discard saved disk data.

Private cleanup uses an explicit directory-descriptor stack, `openat`/`fstatat`
and `unlinkat`; links are unlinked without following targets. Read-only private
copies or committed backups can be made writable for retirement. Every open frame
receives one close attempt, including failed traversal prefixes. If own deletions
invalidate enumeration offsets, an observed `ENOTEMPTY` permits rewinding only after
strict deletion progress; there is no guessed retry count or stale-age rule. Any
uncompleted cleanup retains the receipt/recovery location.

## Bounds and evidence

Finite action, input and physical-entry sets bound all in-memory loops. Inverse
traversal has one action per iteration. Cleanup's explicit stack avoids C recursion;
a successful pass removes at least one remaining entry before a required rewind.
Interrupted syscalls and filesystem progress remain environmental assumptions.
Path/identity matching and the reused preparation checks have the quadratic scan
bounds described in the discovery/staging contracts. Copy/verification costs are
linear in bytes transferred, in addition to filesystem synchronization costs. Memory
holds action/slot/path records and explicit traversal frames plus the temporary
independent compiler snapshots. Source-level allocation failures can leave postcommit
cleanup pending, but cannot cause rollback of an already committed move.

`make -C tests/unit/tools/relocation/transaction/preflight` tests actual independent
lease contention, descriptor identity deduplication/release, invalid recovery placement,
added unused sources, modified importers and removed originals. It fails every measured
lease/revalidation allocation and every lease/check checkpoint, checking both counted
memory and `/proc/self/fd` descriptor restoration.

`make -C tests/unit/tools/relocation/transaction/preparation` tests 8193 binary bytes,
metadata, literal missing-target links, read-only directories, byte corruption,
refused ownership and actual kernel no-replace success/refusal while preserving an
existing target's bytes.

`make -C tests/unit/tools/relocation/transaction/application` exercises a complete
move on one filesystem and between actual different device IDs for the workspace
and `/tmp`, then reloads the final roots with the ordinary compiler frontend. It checks
outside importer/configuration changes, opaque bytes/C comments, empty multiword
directories, cyclic links, inside/outside output retirement and stale-plan refusal.
Every measured application I/O checkpoint and allocation ordinal is refused
individually against fresh fixtures. T113 originally measured 254/1280; T114 adds
persistent preparation/reporting and the tests measure the expanded traces.
Precommit paths/bytes/identities/modes restore; postcommit cleanup preserves the new
project and receipt. A second persistent inverse failure retains saved originals and
both causes. A controlled later inverse completes without rewriting text again.

`make -C tests/unit/tools/relocation/transaction/decision` separately refuses a
commit record acknowledgement and, independently, its retraction. Successful
retraction permits the inverse; failed retraction retains both states and the
record while asserting that no inverse rename was attempted.

Strict compiler/component builds and all four transaction components under
ASan/UBSan pass. Counted allocations/descriptors supplement unavailable leak sanitizer.
Per-file cases and standalone header commands are recorded in
[ready.json](documentation/ready.json); whole-suite/final documentation results belong
to the completed [TODO task](../../TODO.md).
