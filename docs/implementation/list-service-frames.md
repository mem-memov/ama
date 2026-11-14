# List service authority and method result frames

T077 implements the protection/lifetime layer shared by the typed list methods.
The contracts are [LIST-10](../../spec/language/lists.md#list-10),
[LIST-04](../../spec/language/lists.md#list-04) and
[CALL-09](../../spec/language/calls-and-results.md#call-09).
The public implementation boundary is
[`runtime/program/lists/service.h`](../../src/runtime/program/lists/service.h).
The compiler supports [append/insert](list-insertion.md),
[remove/clear](list-deletion.md) and [swap/length](list-ordering.md).
Traversal adapters retain an explicit execution guard.

## One exact grant for the whole service parent

A service frame belongs to the initiating caller. Opening a local list borrows
that caller's existing Running protection. Opening a selected parent's declared
list borrows the exact acquired outgoing parent. Neither path temporarily
releases or reacquires the owner. Arbitrary available owners and matching source
names do not provide this authority.

Admission resolves the declared list once and checks its owner, field selector
and incarnation. The collection then records the exact active frame address as
its linear acquisition token. This runtime-only link is not an Ama reference,
application value or native payload. A copied frame cannot validate because its
address differs. Closing checks the same complete namespace and clears only this
collection's grant; it never restores or releases the borrowed owner acquisition.
Another field's active grant remains untouched.

The frame stays active across every sibling method and its complete output
subtree. At most one method frame is attached at a time. Finishing a method leaves
the parent list busy. Closing while a method is attached, including while its
destructor runs, fails. An ordinary second opening records an explicit Busy
admission refusal on the initiating caller before input/default preparation.
Technical errors do not gain refusal provenance from their numeric status.

## Narrow structural borrowing

Ordinary value selection rejects a busy collection. Only
`ama_program_access_authorized` receives an explicit parent grant for direct
structural input/index/receiver helpers. It validates the active initiating
caller, exact collection namespace, original frame address and still-protected
owner. A different caller, copied grant, other busy field or expired grant cannot
borrow it. Outer result selection and caller-local index operands retain their
separate resolved domains.

No grant is stored on `AmaProgram`, inherited by an executed body or passed to a
native protocol. Existing selection, calls, assignment and transfers use the
ordinary null-authority path. A native conversion failure remains technical;
only the actual busy-list check adds list refusal provenance. Thus a callback
cannot bypass list protection merely because the immediate caller is the same.

## Temporary typed result ownership

A method frame holds one ordinary typed value. A reference result copies its
complete handle without acquiring, cloning or destroying the target. An owned
result is an explicit stable pool root constructed with its selected recipe
after method preflight. Literal or host-size construction does not first create
a default payload and overwrite it. No hidden method instance, C-stack activation
or retained method result is created.

The method frame owns the result's lifetime through output processing. Its owned
root cannot be destroyed by removing an unrelated list membership; it remains
registered for terminal root cleanup. Selected execution still uses normal
instance acquisition. Frame completion consumes the ownership credit before
native destruction, while retaining the parent's attached-method link until
cleanup returns. A callback therefore cannot close the parent or start a sibling
inside that cleanup interval. Constructor failure creates no destroy credit;
destructor failure consumes its one credit and remains terminal. Cleanup may
release the original parent grant after world failure. Escaped nonowning Ama
handles to the owned result become stale when its root is destroyed; retained
native C views are still forbidden by the native borrow contract.

## Stable storage and invariants

The C storage emitter reserves individually typed service/method cells only for
actual lexical intrinsic sites. They follow native payload storage and are not
declared Ama fields or part of native clone payloads. Construction zeroes them;
ordinary and continuation value copies do not inherit active grants. Checked
frame access verifies the current caller identity, alignment and extent before
exposing stable storage. These offsets are trusted generated descriptors, not
foreign pointer inputs.

The central invariant is a bijection between an active service frame and the
collection's acquisition pointer, with equal owner/field/incarnation. Opening
establishes it once; structural access reads it; method transitions preserve it;
closing removes both ends. A second grant cannot be established while the first
is present. The optional attached-method link similarly identifies exactly one
active or currently finalizing method; sibling start/parent close require it to
be absent. Result ownership is a single credit transferred from successful
construction to the method frame and consumed before cleanup. These invariants
give exact acquisition and destruction without an implicit caller privilege or
retry heuristic.

Frame metadata is O(1) per lexical service/method. Each validation performs a
constant number of registry lookups; with the retained linear type registry this
costs O(T), excluding resolved index protocols and native lifecycle work. No
collection target search, payload relocation, per-element activation or global
token registry is introduced.

## Evidence

`make -C tests/unit/runtime/list-service` runs six named cases. `authority.c`
checks uninterrupted sibling protection, copied-frame denial, separate-field
grants, caller operand domains, selected owner retention and alias reentry.
`results.c` checks direct literal construction, stable result identity, stale
escaped handles, nonowning busy targets, constructor/destructor failure,
callback refusal throughout destruction, exact credit consumption and bounded
frame access. `common.c` owns real stable pools, native resources and counted
allocation; `main.c` and its Makefile explicitly run and clean the leaf.

`make -C tests/unit/codegen/list-frame-layout` resolves a real list service with
two sibling methods, emits storage from the actual frontend/scaffold model,
includes the authored native State declarations and compiles/runs a C probe of
all three typed cells. It does not bypass the executable intrinsic guard or claim
that their mutation adapters are complete. Runtime ASan/UBSan, standalone header
checks, per-file coverage, the full strict suite and reproducible Doxygen validate
the layer at its actual boundaries.

[Iteration origin checks](list-traversal-origin.md) add one staged origin tuple for the single owning-clone source in append/insert. Plain reference copies carry no association.
