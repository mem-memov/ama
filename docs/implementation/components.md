# Implementation responsibilities and boundaries

This is T002's engineering map of the implementation audited by
[T001](audit/README.md). The [specification](../../spec/README.md) defines the
language. Generate file/interface descriptions with `make docs`; this map selects
responsibilities, dependency boundaries and test homes rather than maintaining a
second source reference by hand.

## What is retained, changed and removed

The current tree contains working ownership, source, diagnostics, ABI, graph,
planning and stackless-execution foundations with passing old-subset tests.
There is no blanket instruction to delete those foundations. The
[per-file audit](audit/files.csv) assigns every maintained input a decision and
follow-up; [findings](audit/findings.md) identify the specific obsolete contracts.

T136 removed the list action/outcome dispatcher and its consumers; T137 updates
conflicting derived descriptions. Adapt grammar,
identities and other changed contracts in their owning feature tasks. Do not keep
an alternate legacy parser/backend or claim that old input acceptance establishes
current conformance. A missing new feature remains explicitly unsupported.

T138–T144 split reviewed phase-heavy operations, with a commit and tests for each
change. T145–T151 close retained-code documentation/test gaps. New feature work
then follows the queue's context/source/parser/binder/IR/runtime/consumer order.
Directories below marked as planned are destinations, not empty folders to create
before their responsibilities exist.

## Existing boundaries and exact adaptations

A reusable API below is a candidate boundary whose documented domain and tests
must remain valid. It is not approval of every existing body or a frozen public
ABI. Child operations receive only their actual dependencies, not the complete
compiler state for convenience.

| Responsibility and owned state | Existing narrow boundary to retain/adapt | Change and invariants | Independent test home | Real parent composition |
|---|---|---|---|---|
| Allocation arenas and immutable source bytes | `SourceAllocator`, `SourceArena`, `SourceManager`, `SourceSpan` | Manager owns original bytes; spans borrow its lifetime; allocation/length failure publishes no partial source | `tests/unit/source/text` | `tests/integration/source/diagnostics` |
| UTF-8 and XID words | `utf8_decode`, `unicode_identifier_*` | Strict scalar decoding and pinned tables remain; complete multiword names compose words with exact U+0020 | `tests/unit/source/unicode` | `tests/unit/source/module`, `tests/unit/parser/access` |
| Physical catalog and logical roots | `ModuleCatalog`, `module_catalog_find` | Introduce explicit root kind and exact component sequence; preserve byte identity, `name+/`, duplicate/alias rejection | `tests/unit/source/module` | `tests/integration/project/closure` |
| Qualified identity registry | `QualifiedIdentity`, `identity_register` | Encode root kind, category and complete components; preserve full original keys and collision rejection; component qualification is a distinct external boundary | `tests/unit/source/identity` | `tests/unit/native/namespace`, `tests/unit/codegen/assembly` |
| Source diagnostics and rendering | `Diagnostic`, `diagnostic_render`, source adapters | Borrow valid spans; report original bytes and related sites; failure fallback needs no heap allocation | `tests/unit/diagnostics/record`, `tests/unit/diagnostics/render` | `tests/integration/source/diagnostics` |
| Lexical layout | `SourceLayout`, line/payload spans | Validate TAB/spacing before activation; preserve literal bytes and comment indentation/body-closing information | `tests/unit/lexer/layout` | `tests/integration/parser/grammar` |
| Sections, accesses, AST and lifetime | `AstModule`, `parser_module_parse`, ABI parser | Parse current structure without name lookup; explicit iterative frames represent result trees, all control forms and foreach | `tests/unit/parser/module`, `tests/unit/parser/access`, `tests/unit/parser/calls`, `tests/unit/parser/tree`, `tests/unit/parser/abi`, `tests/unit/parser/errors` | `tests/integration/parser/grammar` |
| Project dependency closure | `Project`, `project_load` boundary in `src/project/dependencies/project.h` | One registry per full identity; load each dependency once and preserve requester diagnostics; loading cycles differ from eager construction | `tests/unit/project/closure` | `tests/integration/project/closure` |
| Module namespace and lexical bindings | `SemanticType`/symbol lookup and statement resolution | Explicit inventory plus source-order receiver inference and scoped foreach bindings; no shadowing or runtime-dependent classification | `tests/unit/semantic/names`, planned `tests/unit/semantic/scope` | `tests/integration/semantic/types` |
| Compatibility, capabilities and dependency graphs | Semantic access/transfer and relation-specific graph operations | Nominal type/form checks; separate loading, initialization, eager ownership and greatest-fixed-point clone relations | `tests/unit/semantic/types`, `tests/unit/semantic/relations`, `tests/unit/semantic/constants` | `tests/unit/compiler/check`, `tests/integration/semantic/types` |
| Typed storage/lifecycle/control plans | `ProgramPlan`, `PlannedType`, explicit control nodes | IR carries resolved selectors, action order, failure edges, loop roots, cursor slots and construction choices; emission never guesses them | `tests/unit/planning/ir`, planned `tests/unit/planning/control` | `tests/unit/codegen/assembly`, `tests/unit/codegen/execution` |
| Native ABI, statuses and namespaces | `AmaStatus`, byte views, six ABI records, namespace context | C compiler checks exact signatures/target assumptions; trusted native code owns only native resources and bounded borrows | `tests/unit/native/abi`, `tests/unit/native/metadata/control`, `tests/unit/native/namespace`, `tests/unit/native/toolchain/symbols` | `tests/integration/native/toolchain`, `tests/integration/parser/grammar` |
| Native model and generated interface | `NativeScaffold`, validated type/member descriptors | Separate model construction from emission; generated headers compile independently and preserve module qualification | `tests/unit/native/scaffold/declarations` | `tests/integration/scaffold` |
| Artifact ownership and publication | Scoped ownership model plus frontend-free artifact/IO publication interfaces | Separate preflight/staging/commit/rollback and descriptor lifetimes; never overwrite handwritten files implicitly | `tests/unit/native/scaffold/ownership`, `tests/unit/native/scaffold/filesystem`, `tests/unit/native/scaffold/publication` | `tests/integration/scaffold`, `tests/integration/execution` |
| Context identity and admission, planned `src/runtime/context/` | Context-owned world/program/pool storage, checked host boundary | No mutable global current host; foreign/stale tokens rejected before dereferencing local storage; one executor per context | planned `tests/unit/runtime/context` | planned `tests/integration/context/isolation`, later parallel hosts |
| Stable type pools and checked local handles | `AmaPool`, `AmaHandle`, reserve/publish/invalidate/reclaim | Reserve final identity before initializers; growth preserves payload addresses; retirement/reclamation never revive identities | `tests/unit/runtime/pool`, `tests/unit/runtime/alignment` | `tests/unit/runtime/objects/lifecycle` |
| Ownership construction, cloning and replacement | `AmaWorld`, `AmaRecipe`, `ama_object_*` | Keep initialized-prefix ledgers, subtree preflight and one injective clone map; separate preparation from destructive commit | `tests/unit/runtime/objects/construction`, `tests/unit/runtime/objects/lifecycle`, `tests/unit/runtime/objects/clone`, `tests/unit/runtime/objects/clone-preparation`, `tests/unit/runtime/objects/replacement`, `tests/unit/runtime/objects/access` | `tests/unit/runtime/transfers`, `tests/unit/codegen/execution` |
| Invocation and typed value transfer | Program selection, preparation, acquisition and return interfaces | Resolve once in specified order; preserve caller/callee namespaces, constructor choice, protections and refusal boundaries | `tests/unit/runtime/calls`, `tests/unit/runtime/transfers`, `tests/unit/runtime/program` | `tests/unit/codegen/execution`, `tests/unit/codegen/continuations` |
| Membership lists, planned children under `src/runtime/lists/` | Capacity/checked mutation foundations; new membership/cursor/service contracts | Contiguous order of membership IDs, exact record positions, nonwrapping incarnations, live q corrections and method-specific preflight | existing `tests/unit/runtime/lists`; planned `tests/unit/runtime/lists/membership`, `tests/unit/runtime/lists/cursors`, `tests/unit/runtime/lists/methods` | `tests/unit/codegen/execution`, planned `tests/integration/execution/lists` |
| Explicit control and execution diagnostics | Service metadata, return links, resume positions, source traces | Saved position differs from active caller link; yield releases acquisitions; failure reports before cleanup | `tests/unit/native/metadata/control`, `tests/unit/runtime/calls` | `tests/unit/codegen/continuations`, `tests/unit/codegen/execution` |
| C assembly and native-entry verification | Validated scaffold/plan descriptors plus output stream | One assembly per component; local emission operations take narrow models; context state is materialized separately from immutable code/type templates | `tests/unit/codegen/control`, `tests/unit/codegen/assembly`, `tests/unit/native/toolchain/symbols` | `tests/unit/codegen/execution`, `tests/integration/execution` |
| CLI and reusable compiler stages | `DriverOptions`, `CompilerFrontend`, command-specific orchestration | Recognition, storage, validation and execution are separate; exact options, explicit ownership and shared frontend; no duplicate compiler pipeline | `tests/unit/driver/request`, `tests/unit/driver/options`, `tests/unit/driver/run`, `tests/unit/compiler/check`, `tests/unit/compiler/status` | `tests/integration/launcher`, `tests/integration/check`, `tests/integration/scaffold`, `tests/integration/execution` |
| Standard scalar/refusal/failure library | Declared module interfaces and qualified native hooks | New root identity, required `..integer` bridge, one failure value with description; no action enumeration ABI | `tests/unit/codegen/library`, planned `tests/unit/native/library` | planned `tests/integration/library/contracts` |
| Compiled component exports, planned `src/export/` | Validated export/schema model and generated host adapters | Admission differs from execution failure; public snapshots own data; component/context identities and allocator/code lifetimes remain explicit | planned `tests/unit/export/model`, `tests/unit/export/transport`, `tests/unit/export/identity` | planned `tests/integration/export/c-host`, `tests/integration/export/native-wrapper` |
| Cooperative scheduling/reactor library | Declared task/wait/queue state and native submission tokens | One executor; no hidden Ama references in callbacks; exact epoch/generation match; finite step/import budgets and unique FIFO readiness | planned `tests/unit/async/registration`, `tests/unit/async/ready`, `tests/unit/async/reactor` | planned `tests/integration/async/scheduler`, `tests/integration/async/lifecycle` |
| Parallel native transport, planned `src/runtime/channels/` | Isolated contexts and independently owned typed byte messages | No shared Ama handles/borrows; bounded capacity, explicit admission/backpressure, FIFO and endpoint cleanup | planned `tests/unit/runtime/channels`, `tests/unit/parallel/supervision` | planned `tests/integration/parallel/contexts`, `tests/integration/parallel/channels` |
| Editor/debugger/relocation tools, planned `src/tools/` | Shared source/AST/resolution/inspection contracts | No secondary grammar; rewrite resolved spans only; injective relocation map and staged recoverable transaction | planned `tests/unit/tools/analysis`, `tests/unit/tools/relocation` | planned `tests/integration/tools/relocation`, `tests/integration/tools/recovery` |
| Build, C harness and Doxygen | Recursive source discovery, command manifests, parent/leaf Makefiles, `make docs` | Explicit prerequisites and cleanup; every maintained code file has meaningful tests and generated documentation; no hidden generated-input analysis | `tests/integration/rebuild/tracking`, `tests/integration/rebuild/concurrent`, `tests/integration/harness/lifecycle`; planned `tests/integration/documentation` | Root `make test`, `make docs`, release acceptance |

Existing test paths name actual suites; planned paths name test homes to add
before accepting the corresponding production boundary. Run child suites
independently and test the real parent handoff; merely running a recursive parent
Makefile does not supply a composition test. New test directories need an actual
C `main`/`it_*`/`run` leaf and parent registration, not empty placeholders.

## Dependency and migration order

```text
build/test/documentation checks
    -> retire dispatcher -> retained-component refactoring/documentation
    -> context/native foundations and source identity/layout
    -> structural AST -> closure/bindings/types -> typed IR/lifecycle plans
    -> stable ownership/runtime -> stackless project consumers
    -> complete result/control/continuation/list/library behavior
    -> compiled exports -> asynchronous and parallel consumers -> tools/release
```

The existing compiler is not a second backend while these adaptations proceed.
Change the owning component and its consumers in one testable slice; remove
superseded entry points and registrations. T132 removes retired implementation,
not every reusable component that still needs a future language adaptation.
Tests of valid ownership/ABI/source primitives remain useful even when their old
Ama fixture spelling must be migrated. Tests asserting the old contract are
replaced, not silently retained as current acceptance.

T138–T144 perform the concrete splits listed in the queue. Each split preserves
its verified domain and failure behavior, narrows child dependencies and adds
Doxygen explanations/rule links. T133 reviews whether each retained responsibility
has an appropriate home. T145–T151 then account for all remaining file comments
and tests; T134 cannot close with unassigned debt.

## Contracts and proof obligations

The [contract registry](contracts/README.md) retains useful derivations with their
measured domain and identifies the new boundary obligations. Its
[current boundary contract](contracts/current-boundaries.md) states ownership,
composition invariants and exact validation/test partitions for the changed
models. A future implementation task must complete its operation-level argument
before introducing an algorithm; this map does not certify unimplemented code.

Use complete identity equality, deterministic finite traversals and exact graph
relations. Report an actual semantic ambiguity through `AMBIGUITIES.md`; do not
resolve it by a fallback grammar, guessed field type, callback alias, or old API.
