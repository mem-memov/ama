# Typed direct-result operation trees

T036 resolves direct result operations under
[CALL-09](../../spec/language/calls-and-results.md#call-09).
It supplies semantic identities and ordered action links.
[Execution/protection lowering](direct-result-execution.md) consumes them;
[list interfaces](list-interfaces.md) specialize intrinsic services and
ephemeral method results.

## Two explicit lookup domains

For an ordinary root, the target belongs to the initiating caller. For a selected
operation, the target's base name belongs exclusively to its immediate parent's
result interface. [Target resolution](../../src/semantic/calls/tree/target.c)
retains the parent statement and the original declared result symbol. Parameters,
constants, private fields and same-spelled caller variables cannot substitute for
a missing result. Storage suffixes must match that declaration exactly.

A selected owned scalar retains that instance; a reference retains its referent
selection mode; an indexed owning/reference list retains its selected scalar form.
Selecting a target performs no copy-capability check because it does not copy that
target. Ordinary input/output transfers still require the
[normal transfer rules](typed-transfers.md). Thus a method owning a noncopyable
resource can be called in place while copying that method remains invalid.

All index operands, explicit inputs, output receivers and refusal-handler code
remain in the initiating caller's environment. The
[access resolver](../../src/semantic/values/access.c) accepts an explicit leading
result symbol and resolves the rest of the finite index chain in the caller.
It applies the same exact suffix and owned native index-capability checks used
for ordinary accesses. It does not recursively switch module contexts for indices.

For example, assuming `producer` exposes a `.worker` result `method`, and the
caller already declares `source`:

```text
producer
 method !
  < source
  > received
```

The method target belongs to `producer`; `source` and `received` belong to the
caller. If `received` is a first receiver, ordinary inference declares its field.
A caller variable also named `method` does not change target selection.

## Preserve the action tree explicitly

The source-ordered semantic walk encounters every parent before its descendants.
[Parent lookup](../../src/semantic/resolve/lookup.c) matches exact AST identity,
not source spelling or a dynamic caller chain. Repeated selections of the same
result therefore retain the same declaration with distinct operation records.
Entry, resume and retained modes are independently retained at every link;
absence of a runtime saved position is a future acquisition refusal, not a static
reason to prohibit resume syntax.

After the statement inventory is complete,
[action linking](../../src/semantic/calls/tree/actions.c) builds a typed ordered
list for each operation. Each action points to exactly one checked binding or
selected operation. Binding actions alias the already typed ordinary bindings;
selected actions alias the already typed statement whose parent matches this
operation. No output value or method instance is reconstructed. Interleaving an
ordinary copy between two selections stays visible in this list. Later lowering
must complete a selected subtree before advancing to its next sibling.

A refusal body starts ordinary root operations: its call nodes have no enclosing
selection. Its lexical caller environment and loop context come from the existing
structural walk. The colon remains at its owning target's indentation, and the
handler body is one level deeper. Checking handlers does not acquire any target.

## Invariants and implementation boundary

After each successful statement, its target and every binding have exact nominal
and storage metadata. Every selected statement points to an earlier checked parent
whose result field it selects. After action linking, each source output action has
one corresponding typed link in the same order, and every selected link has the
same parent as its syntax. Partial records remain arena-owned on failure and
`resolved` is published only after all modules finish.

For N statements, A actions, B bindings and D declarations, exact parent/action
lookups cost O(N² + AN + AB), plus interface/name/access resolution. Source nesting
uses explicit collection frames and iterative sorts, not the C call stack. The
new typed statement/action storage is O(N+A); original AST/source owners outlive
all borrowed metadata. The algorithm has no depth timeout, inferred private-field
fallback or guessed target identity.

T044 now lowers the full tree to [explicit frame/action continuations](structured-lowering.md).
[Typed list services](list-service-frames.md) and
[traversals](list-traversal-execution.md) now have executable adapters. T066 implements
[ordinary direct-result execution](direct-result-execution.md), and T068 implements
[ordinary condition headers](condition-loop-execution.md). Static
checking publishes a complete typed plan and never drops selected actions or
turns a selected field into a caller-local access. T063 separately connects self sources to final runtime identity.

## Evidence

Run `make -C tests/unit/semantic/selected`.

The leaf checks all 36 combinations of parent/child entry/resume/read modes and
owned/reference/indexed owning/indexed reference result forms. It inspects repeated
field identity, interleaved ordinary/selected actions, caller-local index/source/
receiver symbols, noncopyable methods and root calls inside both handler levels.
Wrong roles, missing results, suffix mismatches and invalid caller indices fail
without alternative name lookup. A 256-link reference-result tree resolves and
links iteratively. Every semantic allocation in a complete selected tree with
literal inputs, indices, sibling actions and a handler is failed independently;
no partial program becomes resolved and every owner unwinds.

`make -C tests/unit/compiler/check` proves that a valid tree completes semantic and
constant analysis and a complete validated plan before executable storage is
published. Backend emission rejects invalid control operations. Existing
ordinary-call, source-order inference, parser and full-project suites remain part
of the composed verification. These checks certify static selection, not runtime
state changes, protected lifetimes or refusal dispatch of the new trees.
