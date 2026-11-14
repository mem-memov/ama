# Runtime service metadata contract

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This records tested metadata operations under REF-05/REF-06, RT-01 through RT-16,
CABI-05, ERROR-06 and DEBUG-01 through DEBUG-03. Declarations and small checked
operations are in `src/runtime/metadata/`. They fix the engineering representation;
their consumers are the [instance pools](instance-ownership.md) and
[stackless execution](stackless-execution.md). No implicit scheduler is provided.

## Identity, pools and visibility

`AmaHandle` consists of nonzero registered `uint64_t type_id`, `size_t slot_id`,
and nonzero `uint64_t generation`. The canonical empty reference is all zero.
Slot zero is a real slot. Type IDs must be assigned through the collision-checked
full-identity registry, not addresses, basenames or unchecked casts. Exact source
identity encoding belongs to the source/registry stage.

`AmaSlotIdentity` adds live/retired flags. Validation checks generation-zero emptiness,
expected nonzero type, type equality, a resolved existing record, live/nonretired
state, slot equality and generation equality before payload access. Generation
zero always selects the empty-reference case; the all-zero encoding is canonical.
The caller resolves/bounds-checks a
slot ID before passing its record; NULL represents absent/reclaimed identity.
These functions do not dereference a payload or perform acquisition themselves.

Generation advancement accepts a free, nonretired registered slot. Zero advances
to one. UINT64_MAX never wraps: a further advance permanently retires the identity
and reports LimitExceeded. Live slots return Busy without change; retired slots
remain stale. Reclamation must retain identity/generation history outside freed
payload blocks or retire the corresponding identities. No metadata helper resets
that history. Allocation/reclamation and the reduced-width generation limit use
the [stable pool and ownership layer](instance-ownership.md). The pool reuses the
same generation-advance primitive, including permanent retirement at its limit.
`AmaPool` separates block history, allocated payload blocks, live count and free head.

## Per-instance control

`AmaInstanceMetadata` contains identity, the lifecycle ledger and `AmaServiceState`.
Declared interface/body values and opaque native payload are separate typed storage.
No payload mirrors or C activation-stack array are introduced. Activity is Free,
Ready, Running, Waiting, Yielded, Acquired or Failed.

A return link contains attached/host tags, caller handle, return selector and
call-site operation ID. The host root sentinel is attached+host with empty caller
and zero selectors. An unattached connection is canonical zero. An ordinary
connection has a nonempty caller and nonzero continuation/call-site. It describes
one outstanding invocation, not a saved position. Generated dispatch must still
validate selectors against its actual emitted label/source-map tables.

A resume position has its own valid tag and nonzero selector. It is absent in Ready
and present in Yielded. Available instances have no attached return connection.
Successful return/yield must release that connection after protected result
transfer; yield keeps the separate resume position. The generated return dispatcher performs
those transitions, not the metadata declarations alone.

`ama_instance_access` checks external entry/resume/read eligibility and requires published live identity and Live lifecycle state,
then checks activity. Running/Waiting/Acquired return Busy; Free is stale; Failed
cannot resume. Ready/Yielded permit entry and retained read, but only Yielded
permits resume. Invalid modes or inconsistent available-state metadata report
InvalidState. A different instance of the same type is checked independently.
These are pure eligibility checks; actual acquisition, transfer and release belong
to the [program transition layer](runtime-components.md#program-transitions-and-diagnostics).

## Refusal and native service views

The last refusal is an independent `AmaStatus`, initially success. Recording accepts
only the runtime acquisition-refusal reasons; success or a technical error cannot
silently overwrite it. A malformed NULL/length view is also rejected before
replacing any saved field. Normal access/read does not clear it. Nested callers own
their own snapshots. The runtime must call recording before entering `:`; static
constant reads do not call it. An invalid-mode/internal-state failure is technical,
not a new application-visible acquisition reason.

`AmaNativeServices` contains only a value copy of immediate caller refusal, or
success for a host caller. It exposes no pointer into mutable metadata, dispatcher,
instance address or control selector. Message pointers are immutable program-lifetime
text. Native code receives a const operation-scoped view and cannot retain it.
Changing a copied snapshot cannot change its caller's service state.

Service cloning accepts a valid available source and distinct fresh/unused output
storage; failed preflight leaves output unchanged. It builds zero service state,
optionally copies only the resume position, then selects Ready or Yielded.
Caller links, current operation and refusal history remain zero. Identity and
payload/lifetime are outside this operation and cannot be copied accidentally.
This is not a complete ownership-tree clone: the retained
[clone preparation](clone-preparation.md) layer enforces whole-source availability,
ancestry, fixups and publication.

## Source maps and host boundary

`AmaSourceLocation` holds immutable UTF-8 module/path byte views and one-based line
and byte-column. Operation IDs start at one; zero means not selected. The active
operation and each return connection's call-site ID are distinct. Generated maps
must cover them, including initialization diagnostics; no object history is stored.

`AmaHostResult` distinguishes Completed, Yielded and Failed. Normal outcomes have
success status and nonempty root identity; failure has nonzero status and may
have an empty root when initialization failed. Structural message views and root
encodings are checked, including the allocated code domain under the
[checked status contract](native-status.md). `AmaRuntimeMetadata` keeps Idle/Executing/Terminated phase,
root and current identities per host runtime. Failed execution terminates, yielding
returns Idle without a caller connection, and a host explicitly resumes or disposes.
The standard launcher must diagnose an unhandled yield, not spin or succeed.
The [generated host boundary](continuations.md) implements those transitions
using `AmaHostResult`, the current program identity and the host root identity. These are local handles;
structural validation does not authenticate context ownership, and independent
[context foundation](execution-contexts.md) supplies origin-checked access; generated
host wiring is connected by [T061](generated-contexts.md). Public embedding tokens
remain the embedding tasks.
The standalone launcher diagnoses unhandled root yield and disposes the root.

## Justification, effects and evidence

All supplied pointers/views are valid borrowed metadata owned by the synchronous
runtime; native payloads cannot alias it. Each identity/access check is a finite
conjunction/case split and changes no input. A nonzero generation advances strictly
until the maximum and then retires, proving no wrap through zero or stale-reference
resurrection. The implemented pool preserves the same record/history invariant.

Service cloning writes one completed local value only after successful checks,
proving failure preservation and exclusion of caller/history fields. Recording
mutates only the selected caller snapshot. Snapshot creation is a value copy.
Every helper uses O(1) time/space and allocates nothing; none suspends or dispatches
an Ama module. Host validation checks representation consistency, not pool liveness.
Native C can violate contracts, but such code is unsupported rather than silently
reinterpreted as a tracked relationship.

Run `make -C tests/unit/native/metadata/control`. Eight cases cover handle/type/
liveness/generation checks, exhaustion and no revival, busy states and distinct
same-type instances, root/ordinary return links, caller snapshot independence,
ordinary/continuation service copies, resume refusal and host outcomes. This leaf exercises metadata operations only; the
[runtime composition evidence](runtime-components.md#evidence) separately tests
its pool and generated-execution consumers.

The metadata suite and symbol checker also pass ASan/UBSan with leak detection
explicitly disabled for the same ptrace limitation described in the lifecycle
contract. The full suite passes `make -j4 test` with `-DNDEBUG -Werror`.
