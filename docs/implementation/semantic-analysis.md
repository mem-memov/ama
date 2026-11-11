# Semantic analysis and resolved plans

This implements section 5 of [TODO](../../TODO.md) incrementally, under
NAME-04, TYPE-01/TYPE-05 and COMP-01/COMP-02 of the
[specification](../../TECHNICAL_SPECIFICATION.md). The public boundary is
[semantic.h](../../src/semantic/semantic.h). Passing an earlier stage never
certifies an unimplemented later stage.

## Namespace construction

`semantic_program_init` borrows a valid loaded project, which must outlive the
semantic owner. Call `semantic_names` once, then inspect only successful namespace
output. Destroying the semantic program releases its ABI tables and arena before
the caller destroys the project. All declaration/use spans borrow that project.
Allocation failure retains an allocation-free diagnostic and partial owned
records solely for cleanup. No C text, native execution or filesystem writes occur.

[The walker](../../src/semantic/walk/blocks.c) advances explicit frames over AST
blocks; it visits nested alternatives and loop bodies in lexical depth-first
order without recursive C calls. [Namespace construction](../../src/semantic/names/build.c)
visits constants, parameters, results and body in order. Every declaration gets
one base-name symbol, a module ordinal and a section ordinal. Storage suffixes
are attributes, not part of the lookup key; blocks create no shadowing namespace.
[Lookup](../../src/semantic/names/lookup.c) compares original UTF-8 bytes exactly.
Equal names in different module tables do not conflict. A duplicate error retains
both complete declaration spans, including the original storage forms.

Induction over the finite traversal gives the invariant: the table contains
exactly the declarations in the visited prefix, with unique base names and
ordinals in lexical order. Before every insertion, full byte equality either
finds the unique earlier declaration and rejects duplication or proves insertion
preserves uniqueness. Thus successful construction implements NAME-04 for every
storage-form pair and every nesting shape, not just the examples in tests.
Each AST node advances once and every pending frame exhausts a finite block, so
the traversal terminates. With `D` declarations, `N` AST nodes and maximum name
length `L`, namespace construction costs `O(N + D² L)` time, `O(D + N)` owned
storage, plus the existing project. Representable arena sizes bound counts.

[Namespace tests](../../tests/unit/semantic/names/main.c) use the real loader and
parser, enumerate all 16 pairs of storage suffixes, check ordering across sections
and nested alternatives, allow reuse across modules, and inject every semantic
allocation failure with live-allocation cleanup counters. The suite is independently
runnable through `make -C tests/unit/semantic/names`; release flags and `-Werror`
retain assertions through the shared leaf harness.

## Native contracts for the required closure

`semantic_native` requires a successful namespace stage and runs once. It visits
only records in the required closure whose parsed selector is `@C`. Missing
`.h`, `.c` or `.abi` companions fail at the selector, naming the module and missing
suffix. ABI tables use the existing fixed-record parser, preserving duplicate
locations and malformed-record errors. Each table belongs to its semantic type:
identical function spellings in different native modules remain different typed
bindings, never global-name lookup. C bodies and signatures are not parsed or
executed; later generated C uses check handwritten signatures.

The complete selection predicate is `required && selector == MODULE_NATIVE`.
Exhaustion of that finite set establishes all required companion/table checks;
unused draft ABI grammar cannot enter the selection. Existing source catalog
UTF-8/filesystem checks still apply globally. The phase costs the sum of required
ABI parsing costs plus `O(V)` type visits and owns those tables until program
destruction. All exits preserve the table/arena ownership established before
parsing. [Native tests](../../tests/unit/semantic/native/main.c) enumerate all eight
presence masks, exercise equal module-local helper names, unused drafts, duplicate
ABI bindings and every native-parse allocation failure. Release/Werror tests pass.

## Resolved values, interfaces and storage compatibility

`semantic_resolve` follows namespace/native validation. It first links every
value declaration to the unique executable module type; constants-only namespaces
cannot be instance types. It then computes clone capability over all owned
interface/body fields (including list element types), excluding shared constants
and reference edges. Starting with every executable type except native types
without `=`, repeated removal of types owning a removed type computes PROTO-07's
greatest fixed point. No more than `V` types can be removed. This is capability
inference, not an eager-layout cycle test.

[Access resolution](../../src/semantic/values/access.c) matches the full declared
storage form and resolves every named nested index iteratively. Index and condition
conversions require the operand's own native `[]`/`?` binding and an owned scalar.
A reference does not implicitly dereference, nor does a field's capability propagate
to its owner. Mutable access to constants is rejected. Initializer visibility
requires earlier local declarations and restricts shared initializers to constants;
qualified local forward references cannot evade the same ordering.

[Sources](../../src/semantic/values/source.c) retain module identities, original
spans and payload bytes. Qualified sources resolve only exact shared members.
[Transfers](../../src/semantic/values/transfer.c) implement TYPE-05 as a product
of nominal type equality, scalar/list cardinality and ownership form. An indexed
element is scalar storage. A declaration list initializer is checked as one
scalar element, not list splicing. Literal construction selects its destination
owned scalar's `|`; no type or numeric value is inferred from bytes. `%` can clone
an owned/reference scalar into owned scalar storage, subject to clone capability.
Ordinary identical-storage self-assignment is an explicit no-op; `%` self-copy is
rejected. Runtime identity/ownership overlap checks remain runtime obligations.

[Call resolution](../../src/semantic/calls/resolve.c) binds targets, named labels
or independent positional sequences before consumption. Every placeholder counts
against arity, without a hidden receiver/source. Named labels must match the full
field storage form and cannot repeat in their input/output sequence. Bindings
retain text order and resolved field ordinals separately: installation order is
not confused with evaluation/transfer order. CALL-07 literals derive their type
from the selected parameter, with its declaration retained for diagnostics.
Stored-result reads expose results; namespace reads expose constants; neither
makes body/private fields public. Synthetic output values are typed records,
not fabricated parser nodes or C text.

Whole-list calls use LIST-04's four-input/three-output positional schema.
`SemanticLibraries` supplies explicit full module paths for action, index,
condition and outcome roles; paths are copied into the semantic owner. Required
role types must already belong to the loaded closure. There are no hard-coded
English module identities or guessed library protocols. The caller/bootstrap must
load its configured library types along with source requirements; publishing the
standard library and wiring its configuration into the CLI are subsequent tasks.
Missing/invalid mappings fail semantic checking before any emitter. Owning-list
input three requires clone capability whenever supplied, even if unused or empty;
a placeholder permits default creation of a noncopyable element. Reference-list
input three copies handles. Opaque native constant purity/resource restrictions
remain native-library obligations, never guessed from C helper names or payloads.

The invariants of each stage compose: identity links are complete before any use
check; every resolved access names one symbol and recursively well-typed index;
every successful transfer satisfies exactly one declared storage rule; every call
binding names one legal field/position. Induction over each finite source/access
sequence proves completeness, while nominal pointer inequality rejects distinct
module identities regardless of matching representations. Partial arena results
are never published as `resolved`. With `N` nodes/uses, `D` symbols, `V` types,
`E` owned edges and maximum name length `L`, straightforward lookups/checks cost
`O(N D L + N V L + V E)` plus pairwise named-binding checks; memory is linear in
resolved uses, bindings and traversed blocks. Algorithms do not recurse on source
depth and do not execute native constructors at compile time.

[Type tests](../../tests/unit/semantic/types/main.c) enumerate all 32 combinations
of destination/source storage form and same/different module identity. Additional
cases exercise literal/reference/continuation/default rules, immutable constants,
forward initialization, binding arity/labels/duplicates, retained/static reads,
explicit index/condition capabilities, list mappings/clone restrictions, preserved
payload bytes/order and every resolved-allocation failure. Release/Werror checks
and focused ASan/UBSan supplement the derivation; LeakSanitizer remains disabled.
