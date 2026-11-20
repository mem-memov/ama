# Source binding, inferred fields and foreach scope

This boundary implements
[TYPE-06](../../spec/language/declarations.md#type-06),
[NAME-06](../../spec/language/declarations.md#name-06) and
[FLOW-05](../../spec/language/lists.md#flow-05).
Its source records are independent of runtime contents and concrete captures.

## Fix the name before solving its Type

Explicit declarations are collected before source uses are classified. A separate
encounter set records actual caller-variable occurrences. A first simple receiver
may introduce a persistent field; a preceding unresolved use cannot be repaired
by a later receiver. Callee labels, selected member names, module paths, static
names, comments and literals are not caller-variable encounters. Index operands
are ordinary caller uses and retain their own source names.

A later explicit declaration does not suppress an earlier inferred introduction:
that is a duplicate declaration. An ordinary earlier use may resolve a later
explicit declaration under its module-wide availability law. Every source use
retains that original field identity even if its Type still needs equations from
other calls, results or a static export. Immediate concrete Type selection is not
required; rebinding a source name after solving is forbidden.

## One physical event order

[Collection](../../src/semantic/static/connections/runtime/events/collect.c)
retains declarations, initialization sources, assignments, call heads, individual
bindings, control conditions and traversal headers. Its
[iterative sorting](../../src/semantic/static/connections/runtime/events/sort.c)
orders actual source positions before
[binding](../../src/semantic/static/connections/runtime/bind/events.c).

This preserves interleaved parent bindings and nested selected calls: a later
parent argument cannot make a name available to an earlier child. Receiver
classification precedes its adjacent source. Omitted bindings introduce no field.
Accepted source field/use records are immutable; typed connections attach
scoped terms and constraints to them without executing or unfolding callees.

## Type and storage obligations

The complete ordinary storage rules apply to owned/reference scalars,
whole/indexed lists, reference acquisition, self and explicit continuation copies.
Written suffixes and callee interface direction are not inferred conversions.
Native capability, clone and actual transfer checks still run after closed Types
are projected into ordinary semantics.

List services use their fixed typed intrinsic interfaces, including the Standard
integer dependency. They do not instantiate method modules or select actions by
constants. Source dependency extraction retains the actual requiring construct;
missing Standard modules never fall back to Project modules.

## Lexical traversal cells

At a foreach header, resolve the mutable collection and the visible binding name.
A compatible enclosing local or visible module reference is reused. An existing
incompatible name is an error, not an invitation to shadow it. If absent, the
source plan introduces a local reference whose interval is precisely the body.
Nested intervals select the innermost visible original declaration; disjoint
bodies can own separate same-named cells. Later inferred module fields do not
retroactively change earlier local bindings. Creation-time initialization cannot
read runtime foreach-local storage, even inside the body's written source range.

[Source availability](../../src/semantic/static/connections/runtime/fields/visibility.c)
serves editor observations from those same lexical/source records. Typed lookup
and later control lowering retain each original field and exact capture owner.
No second event interpreter is used after Type closure.

## Evidence and failure

`tests/unit/semantic/inference` now prepares real guarded source/Type plans before
checking field forms, interleaved results, selected operations, intrinsic lists,
first occurrences, disjoint/nested scopes and constructor visibility. Its
allocation sweep measures current source preparation and ordinary native/proof
checks rather than the removed event replay implementation. Every refusal keeps
original diagnostics and safely retires partial owners.

Static source/typed connection and specialization inventory suites separately
verify refinable terms, concrete capture distinctions, nested signatures and
projection. See [semantic composition](semantic-composition.md) and the
[T193 record](static-relocation.md) for current source migration and acceptance;
older task receipts do not claim current aggregate conformance.
