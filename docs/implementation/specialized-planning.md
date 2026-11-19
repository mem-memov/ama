# Concrete specialization planning

T187 follows [T185](../../DONE.md#t185) ordinary admission and
[T186](../../DONE.md#t186) constant lifecycle acceptance. The canonical contract is
[COMP-02](../../spec/implementation/compiler.md#comp-02),
[GEN-03](../../spec/implementation/compiler.md#gen-03),
[GEN-04](../../spec/implementation/compiler.md#gen-04) and
[RT-02](../../spec/implementation/runtime.md#rt-02). No specification change is
authorized or required.

## Steps

1. Compose the actual frontend with multiple concrete bindings, repeated equal
   bindings, static exports and unused partial results. Check exact semantic-to-
   planned pointer correspondence, runtime interface ordinals, source provenance,
   all four field forms, shared constants, stable slots and explicit call/resume
   edges. Static producer graphs must not become runtime objects.
2. Test independent malformed-IR mutations and every allocation refusal after
   successful concrete semantic admission. Correct actual consumers, removing
   replaced implementations and tests of only obsolete behavior.
3. Run existing planning consumers and sanitizer checks, document invariants,
   costs and ownership, validate Doxygen/per-file coverage, then archive/commit.
   T188 retains full specialized code/layout emission, including T186's observed
   duplicate ordinary C type-name descriptions.

## Boundary and current findings

Planning borrows the complete immutable semantic graph and its longer-lived
source/static owners. Its arena owns storage, lifecycle and control records;
failed records remain destroyable but cannot be published as valid. A successful
plan maps exact semantic pointers, rather than reconstructing types from source
paths or emitted C names. General stable-slot and explicit service state are kept;
this task introduces no compact representation or saved working-state mirror.

The existing storage pass creates nominal vertices before field links, so cycles
through references and empty lists do not need recursive layout construction.
It already separates shared constants from ordinary writable fields. Validation
checks exact source symbols, type pointers, storage forms, slot ordinals and
control/lifecycle edges. These existing operations still need concrete multi-
specialization composition evidence; their raw-concrete tests alone are not a
claim of template support.

## Component outcomes and invariants

The strict O2/Werror source fixture passed storage admission without production
changes (`/tmp/ama-t187-storage-before.log`). The strengthened five-case leaf also
passed (`/tmp/ama-t187-planning-refusals-before.log`): exact call/interface mapping,
concrete lifecycle order, ordinary versus continuation clone modes, ten independent
cross-capture/inventory corruptions, all 379 allocation positions on the successful
concrete planning trace, and selector exhaustion. After each refused allocation,
plan destruction restores the exact pre-plan live-allocation count while the
admitted semantic/Type graph remains unchanged and valid.

An initial control-test expectation was refused
(`/tmp/ama-t187-control-lifecycle-before.log`): it required an entry node to own a
return continuation. The declared IR instead attaches that continuation to the
explicit SET_RETURN node preceding entry. Each source yield similarly has an
independent SAVE_POSITION/resume edge and shares one type-local YIELD terminal.
The corrected test checks two real call connections and two source yield positions
per worker; it does not impose an unnecessary duplicate-terminal representation.
No production behavior or language rule was changed to satisfy the test.

The storage invariant is a bijection from each admitted SemanticType pointer to
one PlannedType pointer, with one ordered nonconstant StorageField per original
semantic declaration and exact target pointers. Equal complete owner keys already
share a semantic vertex; unequal keys remain separate despite sharing source AST
spans. Allocating all nominal vertices before links preserves finite cyclic graph
structure without recursive layout expansion. Constants use separate immutable
SharedStorage records and acquire no executable service state. Unused partial
exports and static producer contexts remain compiler metadata, outside this
runtime inventory. Per-instance logical fields, cells, traversal frames and
continuation state remain explicit; no state is moved into the static cache.

Lifecycle recipes use those same concrete fields in source order for creation and
reverse order for destruction. Owned scalars/lists clone; references/reference
lists copy handles. Ordinary clones clear positions and explicit continuation
clones preserve them. Only local nodes own normal/refusal/failure/resume edges;
entry edges cross to the exact callee plan. No C names, offsets, textual replacement
or emitter inference supplies any of these identities.

The existing storage constructor uses O(V) nominal records, O(F) field records and
O(K) shared records for V semantic vertices, F persistent fields and K constants.
Its linear exact-pointer lookup gives O(V² + (F + K)V) mapping/layout work before
frame construction and full validation; existing frame/lifecycle/control contracts
own their additional costs. Traversal is finite over admitted owner inventories;
source statements/control cycles are not executed during planning. Validation
allocates no memory, including refusal of all ten deliberately malformed plans.
Selectors fail at SIZE_MAX before increment, rather than wrapping a continuation.

Entry selection is unchanged. The accepted source fixture has exactly one
concrete entry module and two variants only of its worker. This task does not
introduce arbitrary selection among hypothetical entry variants or claim a new
root-inference policy; the compiler root/ordinary-admission contracts remain its
preconditions. No obsolete alternative planner was found: existing implementations
serve actual product consumers and are retained rather than duplicated.

## Scoped acceptance receipts

The strict five-case leaf passed in
`/tmp/ama-t187-planning-refusals-before.log`; the complete six-leaf planning parent
passed in `/tmp/ama-t187-planning-parent.log`. Both commands use
`C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`.
The parent command is `make -C tests/unit/planning -j4`; the independent leaf is
`make -C tests/unit/planning/specializations`.

The same five-case leaf passed ASan/UBSan in
`/tmp/ama-t187-planning-sanitizers.log`, using O1 with the same strict warnings,
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` and
`LINKER_FLAGS='-fsanitize=address,undefined -no-pie'`,
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1. Every 379 successful-trace planning allocation
position was refused independently. Counted resource balance is independent
of sanitizers; LeakSanitizer is unavailable. This evidence covers actual source,
static inference, ordinary admission and resolved planning without generating C;
compiled specialization execution remains T188's separate acceptance owner.

All maintained input changes belong to tests, their build registration, per-file
coverage and versioned task/contracts. No production planning implementation,
canonical specification or conformance fingerprints changed. A Makefile purpose
comment was generalized to avoid a stale enumerated child list; executable Make
content is unchanged after the accepted parent run. The T196 aggregate
specification-review refusal is not presented as full-language conformance here.

Full record: [T187](../../DONE.md#t187). Initial Doxygen/per-file receipt:
`/tmp/ama-t187-docs.log`; exact archive receipt:
`/tmp/ama-t187-archive-docs.log`. Doxygen alone generates the reference pages and
checks real file/interface pages and originating-file links to canonical rules.
T188 is the next ready task.
