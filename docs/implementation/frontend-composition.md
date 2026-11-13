# Complete frontend-to-plan composition

T045 verifies the public frontend against independently invoked stages under
[COMP-02](../../spec/implementation/compiler.md#comp-02),
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[ARCH-05](../../spec/implementation/compiler.md#arch-05),
[ARCH-06](../../spec/implementation/compiler.md#arch-06) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
It extends [semantic composition](semantic-composition.md) through
[structured lowering](structured-lowering.md); execution adapters remain separate
implementation tasks.

## Boundary and ownership

The public `compiler_frontend_check` loads the requested closure, seals its input
snapshot, completes semantics, builds storage/lifecycle/control plans and publishes
success only after plan validation. The test independently runs explicit semantic
stages followed by `program_plan_build` against that same immutable loaded project.
Each path owns separate semantic records and a separate plan arena. The project
and original AST/source objects are deliberately shared: they provide canonical
identities for exact comparisons, rather than comparing unrelated pointer values.

The independent projection compares every resolved type, symbol, source span,
transfer, capability, lexical role and semantic diagnostic. It also compares
storage fields, service masks, context domains, fixed frames, local cells and
origin slots; lifecycle recipes and their failure successors; all control labels,
operands, next/alternative/resume/failure edges; replacement recipes; selected
chains, loop entries/repeats/exits and shared startup/reverse-cleanup order.
Nominal identity is the original canonical module, never a basename or digest.
Unique selector identities and allocation ranks identify independent control and
lifecycle nodes, with null distinct from every real node.

Independent expected assertions establish the scenario itself: exact field/frame
counts, separate reference/local roles, absence of invented intermediate method
variables, byte-exact literals, the typed integer-size bridge obligation and
captured native-header bytes. Positive cases include an empty executable, a
same-type reference cycle and imported immutable namespaces. An unused draft
remains outside the loaded closure.

Destroying the independent plan and semantic owner leaves the public plan valid.
The frontend is then destroyed in dependency order, with counted allocation
balance returning to zero. No test relies on native execution: companion C files
and headers contain `#error`, so accidentally compiling them would fail visibly.

## Failure and authoring boundaries

Real invalid inputs exercise active drafts, layout errors, invalid entry kind,
duplicate/missing native ABI contracts, incompatible transfer forms, traversal
cell types and constant cycles. Project-stage failures publish no semantic owner;
semantic failures publish no plan. Explicit stages and the public path retain the
same diagnostic category, original source/related spans, prose and attempted
stage prefix. Partial outputs remain destructible.

Native authoring supplies an explicitly owned ABI proposal for precisely the
entry module. The proposal is retained in the input snapshot, participates in
semantic comparison and produces a validated plan. A separate strict frontend
against the same invalid on-disk ABI fails at its original file instead of
silently borrowing that proposal. This proves the authoring context does not
contaminate ordinary checking.

Every allocation of a complete public frontend run is failed in turn. Failures
must report out-of-memory, preserve a valid diagnostic, publish no successful
plan and release every allocation, including failures after semantics and during
plan construction.

## Composition argument and verification

Each stage is a partial function over successful predecessor output. Induction
over the fixed stage order proves that a successful public result consists only
of complete validated predecessor results. On failure, short-circuiting preserves
the first failed stage; ownership credits cover exactly the initialized prefix.
The public composition adds a fixed number of stage dispatches, so its time and
space bounds are the sum of the component bounds plus input-snapshot accounting.
The exact projections are test oracles; their intentionally exhaustive scans are
not compiler runtime algorithms or claims of linear complexity.

Run `make -C tests/unit/compiler/composition`. The four named scenarios cover
positive projection, invalid-project diagnostics, isolated native authoring and
allocation-prefix cleanup. The [coverage registry](documentation/ready.json)
maps the new leaf and reused comparison/build infrastructure to these cases.
Full strict tests, address/undefined-behavior sanitizer checks, standalone-header
coverage and reproducible Doxygen generation complete the task evidence.
