# Source declarations and concrete interface views

This implementation boundary serves
[NAME-04](../../spec/language/declarations.md#name-04),
[NAME-06](../../spec/language/declarations.md#name-06),
[TYPE-06](../../spec/language/declarations.md#type-06) and
[CALL-08](../../spec/language/calls-and-results.md#call-08).

## Original names precede concrete Types

[Static inventory collection](../../src/semantic/static/inventory/collect.c)
records module-local static inputs, graph results/aliases and complete original
syntax. Static producers have public source names without ordinary runtime fields.
[Runtime source preparation](../../src/semantic/static/connections/runtime/prepare.c)
collects explicit ordinary declarations and classifies inferred receivers,
foreach cells and uses in physical source order. Neither component creates a
runtime instance or guesses a Type from a partial spelling.

All ordinary storage forms share one complete base-name namespace. Original
field records retain role, declaration/source identity, storage suffixes and local
scope. Forward explicit declarations and source-ordered inferred introductions
remain distinct availability laws. Type equations may be solved later, while
already chosen source binding identities never change.

A constructor capture instantiates a concrete Type descriptor. The
[projection](../../src/semantic/specializations/inventory/build.c) registers every
required owner by its full key before linking fields. Multiple captured owners
retain distinct SemanticType/SemanticSymbol pointers but share original field
identities. Static producers remain source graphs; one source module is not one
concrete Type. Closed capture environments supply field Types, defaults and local
references without repeating lexical classification.

## Role-specific interfaces

[Exact declaration lookup](../../src/semantic/names/lookup.c) compares complete
UTF-8 names. [Interface lookup](../../src/semantic/names/interface.c) keeps input,
result and constant roles and their own source ordinals distinct. Named and
ordinary positional bindings consume those same accepted field records. Private
body state and foreach locals cannot become public parameter slots. Static
interfaces use named Type/Template bindings instead of runtime positions.

Graph names, qualified constant member names and caller variable occurrences
belong to their own source namespaces. Module paths resolve actual dependencies;
atomic three-dot names do not search the filesystem. No path-only lookup repairs
a missing concrete capture or an absent source namespace proof.

## Verification

`tests/unit/semantic/names` retains duplicate-form and Unicode/base-name cases,
source-order declarations, actual AST walker ordering and allocation cleanup.
Its duplicate matrix isolates permitted declaration forms: a reference constant
is independently invalid and cannot impose a duplicate-error priority.
`tests/unit/semantic/specializations/inventory` checks distinct captures,
source-bound inferred/local fields, all section roles, defaults, finite nominal
cycles and every projection refusal. Runtime/static source connection suites
verify the original lexical decisions before solving.

The production semantic walker remains independently tested with parsed AST and
an explicit syntax owner; an intentionally untyped walker fixture does not claim
a valid closed Type. Concrete-only inventory comparisons that required unresolved
operation names were removed with their replaced implementation. Current
[semantic composition](semantic-composition.md) and [T193](static-relocation.md)
record the shared source preparation, ownership and migration evidence.
