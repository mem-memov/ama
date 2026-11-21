# Continuation forms and positive copy-boundary proofs

T039 checks [CONT-03](../../spec/language/continuations.md#cont-03),
[CONT-04](../../spec/language/continuations.md#cont-04),
[CONT-05](../../spec/language/continuations.md#cont-05) and
[COPY-06](../../spec/language/values-and-references.md#copy-06).
Runtime availability, actual indexed identities and full ownership-tree checks
remain mandatory. [Saved statement positions](saved-statement-positions.md),
[continuation cleanup](continuation-cleanup.md) and
[traversal copying](list-traversal-execution.md) supply real control-frame consumers.

## Three operations, three explicit meanings

Standalone body `%` is suspension. Suffix `task %` is resumption. Prefix `% task`
is a continuation-copy source. None introduces a new nominal continuation type.
A source with no saved root position is still a legal copy source; preserving
absence does not create a resumable root. Ordinary module types can own resumable
children even when their root has no saved position.

Resume rejects every input line, including an empty `<`, before target access.
It accepts result transfers or no bindings and performs no entry reset. `>>`
belongs only to a nonexecuting read. A literal beginning with `%` after `|` remains
literal bytes, not suspension or a copy modifier. Parser and semantic stages do
not infer mode from target names, body contents or a supposed saved-position flag.

The [transfer checker](../../src/semantic/values/transfer.c) keeps these choices
explicit:

| Source/operation | Typed transfer |
| --- | --- |
| Ordinary owned scalar/list value | Independent ordinary clone |
| Reference scalar/reference-element list | Handle transfer |
| Prefix `%` on owned/reference scalar | Independent owned continuation clone |
| Literal input/initializer | Literal construction of the selected owned scalar |

Nominal identity and storage forms must match. Prefix `%` has an owned scalar
destination; it does not initialize a reference cell or copy a whole list. An
individual initializer of an owning list is a scalar destination and can clone
one source with `%`. The complete owned tree below that scalar, including lists,
then belongs to the continuation-copy boundary. Shared constants and qualified
constant namespaces are not continuation/reference-acquisition operands.

Every owned transfer requires static clone capability, including apparent
self-assignment. Ordinary independent copies clear saved positions; reference
transfers preserve their targets' state. The normal runtime exact-identity no-op
is a separate COPY-06 exception after access checks and is never a static
capability exemption. Explicit `%` requires an independent destination and cannot
use that no-op exception.

## Retain positive handle origins

[Origin records](../../src/semantic/copy/boundaries/origin.c) describe a proved
reference to one direct owned field, or to the current instance. They retain the
original `& field`/`&` source site and exact symbol identity. A scalar reference
store copies a known origin, replaces it with a newly formed origin, or forgets
it when its source origin is unknown. Empty/runtime handles are not guessed.

For example, assuming copyable `.value`, this body is rejected statically:

```text
@!
value
.value
alias&
 & value
value
 % alias&
```

TYPE-06 gives `alias&` its ordinary persistent reference field before execution.
The body assignment stores the handle of `value`; cloning that referent back into
`value` violates explicit continuation independence. Further reference-handle
copies preserve the same positive proof. Reporting retains the destination's real
declaration/receiver and the original reference-formation site without allocating
fallback diagnostic storage.

[Segment analysis](../../src/semantic/copy/boundaries/analyze.c) follows exact AST
sibling links, not adjacent text lines. A new block/branch arm starts with unknown
origins. Calls, control nodes, suspension, return and operations outside the
modeled handle-only effects discard facts. An unknown reference store forgets
only its destination's origin. Declarations execute their initialization before
the body, so their written body positions introduce no effect between handle
stores. Comments likewise add no statement. Defaults are not seeded as current
body facts: permanent fields and suspended interfaces are runtime state.

This is a positive-proof pass over explicit effect-free segments, not an
interprocedural inference of native behavior or a claim to reconstruct every
runtime alias. No constructor, C source spelling, field name convention or
currently empty list supplies a guessed origin.

## Prove ownership without guessing an index

[Boundary checking](../../src/semantic/copy/boundaries/check.c) rejects two
positive cases before executable planning:

- a known reference to a direct owned field copied with `%` back to that field;
- a known current-instance reference copied with `%` into its owned field/owned
  list element.

The second proof does not need the element position. Every successful owning
selection from that field is a descendant of the current instance. PROTO-04
forbids observable side effects in index conversion, so conversion cannot change
the freshly proved handle origin. A reference-list selection is not an owned
destination for `%` and cannot enter this proof through a type conversion.

Other indexed identities remain runtime state even when source expressions have
the same spelling. Distinct owned peers may be copied. Non-owning links between
them do not enlarge ownership boundaries or make external targets require native
clone support. Runtime must check actual identity, ancestry and source availability
before clone hooks or destructive replacement; it must not create a temporary
snapshot to make forbidden overlap work. A copy failure has the technical failure
protocol, never an invented assignment `:` handler.

## Invariant, bounds and publication

At each modeled segment point, every retained fact is conditional on normal
completion of preceding operations: the cell contains the exact current owned
field/current-instance handle. Reference formation/copy invokes no native clone
or target acquisition under REF-07. Symbol identities belong to this caller
instance; no call or scope transition carries facts into a different environment.
A store propagates only an already proved identity, and every forgotten case
retains dynamic validation. Each reported violation therefore follows from actual
ownership/identity rules, not a possible-alias guess.

[Resolution](../../src/semantic/resolve/program.c) runs the proof pass only after
complete type, binding and control checks, before publishing `resolved`. All proof
records borrow original semantic/source owners and belong to the semantic arena.
For N statements, exact lookup costs O(N²) time in the worst case and O(N) total
proof-record storage; no recursion, probabilistic membership or timeout is used.
Allocation failure leaves the program unresolved and every partial record owned.

## Evidence

Run `make -C tests/unit/semantic/continuations`. Tests distinguish all transfer
kinds, owned/reference scalar sources, incompatible reference/list destinations,
individual list initialization, explicit continuation input copying and ordinary
entry/resume/retained outputs. Invalid prefix/resume/assignment-handler syntax
fails during parsing; literal `%` bytes remain intact. Noncopyable source types
fail even when an identity no-op would otherwise be possible.

Positive proofs cover direct aliases, handle chains, declarations between stores,
branch-local proofs and current-root/owned-element overlap, with exact origin
notes. Branch joins/arms, unknown native output handles, suspension, unknown stores,
initializers and indexed selections retain runtime checking. External reference
resources do not block otherwise valid peer copies. Each allocation in successful
and rejected origin chains is independently failed and counted owners return to
zero. Existing generated execution and runtime copy suites still test the dynamic
boundary and saved-position behavior independently of these new static proofs.
