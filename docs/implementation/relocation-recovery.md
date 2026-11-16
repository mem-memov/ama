# Recovering and reporting a relocation

The relocation report and recovery components implement T114 under
[TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04). They extend
[T113's application engine](relocation-transaction.md): recovery consumes retained
filesystem records, without the compiler snapshots or application process that
created them. T153 supplies the [explicit utility commands](relocation-commands.md); combined
client/project-build acceptance remains T115.

A move has two outcomes. Before commit, recovery restores original locations.
After commit, recovery keeps the published project and retires saved originals.
A terminal receipt records completed cleanup, making another recovery a historical
read even if the user has since edited or removed project files. A failed operation
retains its report, diagnostic, journal and unresolved originals; destruction does
not retry disk operations.

## Ownership and the client boundary

`src/tools/relocation/report/` owns `RelocationReport`. Its root arrays, exclusions,
entry identity, module mappings and physical file rows are deep copies of a ready
staging plan. The original compiler owners can expire without invalidating the
report. Physical rows distinguish rewritten sources/settings, preserved drafts,
opaque entries and explicit output invalidations. An absent output has a separate
`existed` flag; its absence is not a missing source. All completed relocations
require rebuilding their affected generated outputs.

`RelocationTransaction` builds its own report before acquiring application leases.
`RelocationRecovery` independently owns its transaction metadata, report and source
manager. Decoded replacement payloads borrow that source manager, which expires
after transaction metadata. Initialize either owner, perform one attempt, inspect
its result and destroy it. Allocator authority survives destruction. Request
callbacks are synchronous, never execute Ama and do not modify project state.

For recovery, supply the exact existing private journal directory and an optional
fault adapter. `ready` means the complete persisted scope has been decoded and
validated; `finished` means the required outcome and cleanup are acknowledged.
`transaction.committed` distinguishes committed publication from rollback, even
when a later cleanup fails. A false return cannot be presented as successful
relocation. Report paths describe the complete requested mapping; the outcome
flags determine whether it was applied or rolled back.

## Persist preparation before filling private copies

The initial recovery directory is created privately in the explicitly supplied
recovery storage. Before creating other slots, write and synchronize `intent`:

1. The version marker, complete independent report and initial directory identity.
2. The complete `BEGIN` marker.
3. For each subsequent slot, a complete `SLOT` record and end marker, synchronized
   before creating the directory.
4. An `OWN` record containing its exact physical identity, synchronized before
   copying any file or directory into the slot.

A slot name combines the journal's device/inode identity and a checked increasing
ordinal. Existing names and explicit public locations are skipped before reservation;
creation still uses exclusive `mkdir`. This is a finite search through occupied and
reserved locations, with explicit integer-exhaustion failure. Recovery reads the
recorded names; it does not scan for temporary-looking names or steal old locks.

A complete reservation without an acknowledgement may own only an absent or empty
private directory. A nonempty unacknowledged directory causes refusal and data
retention. An acknowledged slot may contain an incomplete prepared copy, but its
recorded device/inode, directory type and private ownership must still match.
Preparation-only recovery never restores or removes a public source path.

The complete initial scope is mandatory. An interrupted initial header has not
authorized any nonempty copy or source mutation; recovery rejects it and retains
the directory. This avoids guessing a missing scope or ownership record.

After all replacement data and directories are verified, the engine writes and
synchronizes `plan`, then appends and synchronizes `READY` in `intent`. Publication
cannot begin before `READY` returns successfully. Without complete `READY`, a
recovery attempt retires only registered preparation copies and writes `ABORTED`.
An incomplete final preparation append grants no additional authority. The parser
accepts only a proper incomplete record prefix; malformed complete fields still
reject. Before writing an abort receipt, truncate the unfinished suffix to the
last complete record offset.

## An exact prepared inventory

`plan` uses the private version marker `AMA-RELOCATION-PLAN-2`. This is an internal
recovery format, not the language grammar or public C ABI. Unsupported versions
refuse. It contains the same report and registered slots as the earlier intent,
followed by ordered inverse actions and `END`.

Integers use canonical decimal spelling and LF: no plus sign, padded zero,
negative zero, optional whitespace or unchecked arithmetic. Text is byte-length
framed and cannot contain NUL. Binary payloads have their own length framing and
preserve NUL, LF and all other bytes. Nullable identities have an explicit presence
bit. Counts are bounded by representable allocation size and the actual remaining
framing bytes before allocation. Metadata consists of explicit fields, not native
`struct stat` bytes or padding; conversions must round-trip through their target
types, and timestamp fractions must be in `[0, 1000000000)`.

The inventory records original/prepared device and inode identities, exact slot
paths and ordered remove/create/replace actions. Changed replacement bytes are
stored in full. Unchanged opaque bytes remain in their saved original tree, rather
than being duplicated in the journal. Prepared tree rows retain both original and
prepared entry identities. Recovery never reruns source rewrites, compiler binding,
constructors, native code, configuration interpretation or shell commands.

Decode the complete prepared prefix before filesystem replay. Validate canonical
absolute physical names, module identity syntax, private slot parent/child geometry,
nonoverlapping public actions, and the ordered source removal/destination creation.
Every outside action must correspond to a reported rewrite or explicit invalidation.
Prepared tree names must be the exact component-wise source/destination projection;
parents precede children, names are unique, and every retained selected entry is
present. The root identity agrees with the create action. Intent and plan reports
and registered slot sets must agree exactly.

## Resolve the decision without guessing

After synchronizing all published parent names, application appends and synchronizes
`COMMIT`. The full marker selects committed cleanup. An incomplete proper prefix of
that append is uncommitted. Unknown or conflicting suffix bytes reject; a timestamp
or the apparent presence of D is never a commit decision.

An application whose commit acknowledgement fails must durably retract the
provisional marker before restoring originals. If retraction fails, it retains
both states and reports an incomplete transaction. A later recovery reads the
actual retained marker and follows that outcome. Before an uncommitted inverse,
recovery durably truncates any unfinished decision/finalization suffix to the
prepared offset.

After cleanup, append and synchronize `DONE-COMMIT` or `DONE-ROLLBACK` in `plan`.
Preparation-only cleanup uses `ABORTED` in `intent`. An incomplete final marker
means retrying the same outcome, never choosing the other one. A complete terminal
marker returns the historical report after synchronizing that receipt, without
examining or changing former public action paths. Receipts remain in the requested recovery storage.

## Grant filesystem authority before replay

Recovery leases the journal's existing parent and directory before loading records.
Records must be current-user private regular single-link files. The loaded identity,
size and modification/change times must remain stable. The recorded initial journal
directory must match the directory actually opened; moving or copying a receipt to
an unrelated directory does not transfer its cleanup authority.

Before mutation, resolve each action parent through exact ordinary directory
components and acquire its exclusive nonblocking lease. Reject redirected parents,
symlinks and contention. Validate every remaining slot's recorded identity and
reserved immediate child names. Already retired slots may be absent. These are the
same Linux/cooperating-writer preconditions as T113; locks are descriptor-backed,
not inferred from process age or a filename.

For uncommitted actions, check the whole inverse domain before starting any inverse:

- Each original is either at its original public path or in its exact saved slot.
- A prepared replacement may be private, published or already withdrawn; a foreign
  public object is never overwritten.
- Published replacement files must retain exact prepared bytes. For a published
  tree, verify every recorded entry identity, permissions/ownership, literal link
  target and regular byte sequence. Enumerate each directory's immediate children
  to reject added unrecorded data as well as missing entries.

Changed source/settings bytes compare with inline journal payloads. Unchanged opaque
bytes compare with retained originals. Links are read literally, including missing
targets and cycles. A matching inode alone is insufficient to discard a modified
published file. A changed or extra published entry retains both states for explicit
repair and retry.

Before committed cleanup, recovery synchronizes the read commit record. A full
marker may still be cached after an earlier executor's failed acknowledgement;
its presence alone cannot justify dropping the last originals before it is durable.
Terminal receipt replay likewise synchronizes its record before acknowledging
completion, including a preparation-only abort receipt. A failed acknowledgement
returns false and leaves `finished` false while preserving the known commit flag.

Committed recovery does not withdraw public replacements or require them to retain
their commit-time contents. Legitimate postcommit edits belong to the new project.
It validates and retires only remaining owned private state. Neither committed
cleanup nor terminal receipt replay reapplies source edits.

## Invariants, termination and cost

The preparation invariant is that copied data can enter only a slot whose name and
identity were durably registered first. Source mutations require the complete
prepared plan and `READY`. The publication invariant is that each action's original
remains recoverable until the durable commit decision. Before commit, reverse-order
inverse probes distinguish actual object locations after interruption; already
restored originals and already withdrawn replacements are no-ops. Restored public
parent names are synchronized before private copies retire. All retired private
slot parents are synchronized before a terminal marker, including already absent
slots whose previous removal may not have been acknowledged.

After commit, inverse execution is forbidden. Cleanup only removes matching owned
private directories, and a failed cleanup retains its receipt. Terminal markers are
written only after required cleanup, so terminal replay cannot affect a newer
workspace. A failed operation's memory prefix is always independently destructible;
owner destruction closes each acquired descriptor once and performs no disk cleanup.

All record loops consume bytes or fail. Array sizes and arithmetic are checked.
Tree/slot validation scans finite explicit sets; cleanup uses T113's iterative
no-follow traversal. Pairwise path/set checks and child-to-inventory matching have
quadratic worst-case record bounds. Reading/copying/verifying bytes is linear in
their total size, in addition to filesystem synchronization costs. Memory holds
report/path/action records, loaded journals, explicit traversal frames and exact
byte snapshots; no compiler analysis state is required for recovery.

## Acceptance evidence

The independent leaves are:

- `make -C tests/unit/tools/relocation/report`: complete scope after compiler owners
  expire, exact roles/settings and every measured report allocation refusal.
- `make -C tests/unit/tools/relocation/recovery/decoding`: a real writer inventory,
  every truncated prepared prefix, all decoder allocation failures, canonical
  integer extremes, NUL/text/binary distinctions, timestamp bounds and exact
  decision/terminal tails.
- `make -C tests/unit/tools/relocation/recovery/replay`: both outcomes, repeat after
  later edits, modified/extra published entries, active-writer contention,
  nonempty unacknowledged reservations, and real cross-filesystem recovery with
  opaque binary assets, cyclic links, drafts, settings and output invalidations.
- `make -C tests/unit/tools/relocation/recovery/interruption`: terminate actual
  child executors at every measured registered writer checkpoint and every
  recovery checkpoint for both commit and rollback. Recover from disk through
  independent owners and recover a second time. The initial accepted
  trace is measured dynamically, including retirement-parent synchronization.
- `make -C tests/unit/tools/relocation/recovery/failures`: every measured recovery
  I/O/allocation refusal, descriptor/live-allocation restoration and independent
  successful retry. Both pending and terminal receipts are tested for commit and
  rollback. Each trace prints its measured I/O/allocation count; assertions do not
  use fixed ordinals.

T113's application, inverse, lease and decision suites remain regression gates.
These tests establish process-interruption behavior on the actual Linux workspace
and `/tmp` devices. They do not simulate sudden power loss or establish arbitrary
filesystem crash-consistency guarantees beyond the documented synchronized ordering.
Per-file evidence and standalone header commands are in
[ready.json](documentation/ready.json); final suite/sanitizer/documentation evidence
belongs with the completed [T114 entry](../../TODO.md).
