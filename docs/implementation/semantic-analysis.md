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
