# Failure is not a saved continuation

T073 composes continuation failure and cleanup under
[CONT-04](../../spec/language/continuations.md#cont-04),
[CONT-05](../../spec/language/continuations.md#cont-05),
[CONT-06](../../spec/language/continuations.md#cont-06),
[COPY-02](../../spec/language/values-and-references.md#copy-02),
[LIFE-02](../../spec/implementation/native-interface.md#life-02) and
[LIFE-05](../../spec/implementation/native-interface.md#life-05).
The existing private clone ledger and terminal context cleanup supply the
implementation. New compiled cases expose completed credits, failed partial
resources and per-instance saved positions through native effects.

## Unwind prepared values before terminal cleanup

One yielded source owns two native payloads with independent 16-byte resources.
Its body advances their counters to one and two. A distinct destination still
owns two default resources at zero.

During explicit copying, the first native clone allocates successfully. The second
allocates its partial resource, frees that resource itself and reports failure.
Only successful raw construction owns a runtime destruction credit, so the
failed second callback must not receive a destructor. The prepared first copy
does own a credit and must be destroyed before any public replacement commit.

The compiled trace is deliberately ordered:

| Effect | Meaning |
|---|---|
| `clone=1`, `clone=2`, `partial=2` | First copy completes; second cleans its own partial resource. |
| `drop=1` | Unwind the successful private prefix. |
| `drop=0`, `drop=0` | Terminal cleanup sees both unchanged old destination values. |
| `drop=2`, `drop=1` | Clean original source values in reverse field order. |

Native hooks check final self-address and resource markers, free before reporting
destruction and reject a missing resource. An unearned destructor, shallow
resource clone or early destination destruction cannot satisfy that trace.

The explicit host observes the original `Native clone failed` diagnostic and a
failed outcome. Its context has been disposed; later resume rejects and repeated
cleanup retains the primary status without running the source continuation.
Availability of a source before the failed copy does not make the failed
execution itself resumable.

## Each node keeps its own completed or yielded state

A completed wrapper can own a yielded child. Explicit copying preserves the
child's position while the copied wrapper still has no position of its own.
Resuming that root refuses NoResume, but directly resuming its retained child
succeeds independently of the source child.

Conversely, a yielded wrapper can own an already completed child. Copying retains
only the wrapper position. A child resume refuses and releases its attempted
operation tree; the wrapper's own position remains available afterward. Source
and copied wrappers then resume independently.

These paired cases prohibit promoting or clearing an entire tree from only its
root's state. The scalar snapshot projection applies the permitted state of each
owned node separately.

## Native code has a synchronous boundary

A native declaration containing an Ama `%` body statement fails frontend source
analysis. Its deliberately uncompilable C companions are never reached. Native
entry remains the synchronous status interface; it exposes no native yield or
C-stack capture operation.

Another compiled native entry returns a valid nonzero NoResume status and returns
normally from C. That code is an admitted technical failure here, not an
acquisition refusal or a request to save a native activation. The host observes a
failed, disposed context, and later resume rejects. Numeric coincidence with a
continuation-related refusal code cannot invent a yield protocol.

The [whole-source availability tests](scalar-continuation-copies.md) separately
reject Running/Waiting owned state before clone callbacks. Native authors still
own their declared resource/clone contracts; this boundary does not sandbox
arbitrary C memory operations or retain borrowed native views beyond their call.

## Evidence

`make -C tests/unit/codegen/continuation-cleanup` runs four explicit cases: one
frontend rejection and four compiled programs spanning mixed positions, resource
clone failure and native status failure. Standard C allocation/string declarations
are now explicit in the shared native fixture writer so its resource fragments
have complete dependencies. The full strict suite covers that shared change.
Target sanitizer flags instrument the generated runtime and native hooks.

Independent runtime protocol/clone suites retain their allocation-failure and
raw-success completion-credit proofs. Membership-aware list/cursor copies remain
their later T084 boundary; no native stack, active caller link or implicit retry
mechanism is added.
