# Implementation responsibilities and boundaries

The [specification](../../spec/README.md) defines the language. This current
component map routes implementation responsibilities, invariants, independent
tests and real parent consumers. Doxygen generates file/interface reference
pages through `make docs`; this document is architecture and contract navigation,
not a manually maintained alternative source reference.

The immutable [T001 audit](audit/README.md) records the starting implementation.
T136 removed the old action/outcome dispatcher; T138–T144 split phase-heavy
operations, and T145–T151 closed retained-code documentation/test debts. Feature
tasks then connected the current context, grammar, IR, runtime, export,
asynchronous/parallel and tool consumers. T116 reconciles this map with actual
production boundaries and test homes. Release conformance remains the explicit
requirement/acceptance matrix rather than an inference from historical tests.

## Current boundaries and invariants

Child operations take their actual dependencies and expose narrow ownership and
failure contracts. A passing parent Makefile is not itself a composition test:
the parent consumers below exercise actual stage/runtime/native handoffs.

| Responsibility and owned state | Production boundary | Preserved invariants | Independent test home | Real parent composition |
|---|---|---|---|---|
| Allocation arenas and immutable source bytes | `SourceAllocator`, `SourceArena`, `SourceManager`, `SourceSpan` | Manager owns original bytes; spans borrow its lifetime; allocation/length failure publishes no partial source | `tests/unit/source/text` | `tests/integration/source/diagnostics` |
| UTF-8 and XID words | `utf8_decode`, `unicode_identifier_*` | Strict scalar decoding and pinned tables remain; complete multiword names compose words with exact U+0020 | `tests/unit/source/unicode` | `tests/unit/source/module`, `tests/unit/parser/access` |
| Physical catalog and logical roots | `ModuleCatalog`, `module_catalog_find` | Introduce explicit root kind and exact component sequence; preserve byte identity, `name+/`, duplicate/alias rejection | `tests/unit/source/module` | `tests/integration/project/closure` |
| Qualified identity registry | `QualifiedIdentity`, `identity_register` | Encode root kind, category and complete components; preserve full original keys and collision rejection; component qualification is a distinct external boundary | `tests/unit/source/identity` | `tests/unit/native/namespace`, `tests/unit/codegen/assembly` |
| Source diagnostics and rendering | `Diagnostic`, `diagnostic_render`, source adapters | Borrow valid spans; report original bytes and related sites; failure fallback needs no heap allocation | `tests/unit/diagnostics/record`, `tests/unit/diagnostics/render` | `tests/integration/source/diagnostics` |
| Lexical layout | `SourceLayout`, line/payload spans | Validate TAB/spacing before activation; preserve literal bytes and comment indentation/body-closing information | `tests/unit/lexer/layout` | `tests/integration/parser/grammar` |
| Sections, accesses, AST and lifetime | `AstModule`, `parser_module_parse`, ABI parser | Parse current structure without name lookup; explicit iterative frames represent result trees, all control forms and foreach | `tests/unit/parser/module`, `tests/unit/parser/access`, `tests/unit/parser/calls`, `tests/unit/parser/tree`, `tests/unit/parser/abi`, `tests/unit/parser/errors` | `tests/integration/parser/grammar` |
| Project dependency closure | `Project`, `project_load` boundary in `src/project/dependencies/project.h` | One registry per full identity; load each dependency once and preserve requester diagnostics; loading cycles differ from eager construction | `tests/unit/project/closure` | `tests/integration/project/closure` |
| Module namespace and lexical bindings | `SemanticType`/symbol lookup and statement resolution | Explicit inventory plus source-order receiver inference and scoped foreach bindings; no shadowing or runtime-dependent classification | `tests/unit/semantic/names`, `tests/unit/semantic/inference`, `tests/unit/semantic/selected` | `tests/integration/semantic/types` |
| Closed ordinary specializations | `semantic_specialization_names`, `semantic_specializations_check`, exact frontend root selection | One vertex per full key; original fields/ordinals; separate clone/eager relations; local checks before finite checked groups; no runtime template instance | `tests/unit/semantic/specializations` | `tests/unit/compiler/specialization-composition`, `tests/unit/library/resolution`, `tests/unit/editor/workspace` |
| Compatibility, capabilities and dependency graphs | Semantic access/transfer and relation-specific graph operations | Nominal type/form checks; separate loading, initialization, eager ownership and greatest-fixed-point clone relations | `tests/unit/semantic/types`, `tests/unit/semantic/relations`, `tests/unit/semantic/constants` | `tests/unit/compiler/check`, `tests/integration/semantic/types` |
| Typed storage/lifecycle/control plans | `ProgramPlan`, `PlannedType`, explicit control nodes | IR carries resolved selectors, action order, failure edges, loop roots, cursor slots and construction choices; emission never guesses them | `tests/unit/planning/ir`, `tests/unit/planning/structured`, `tests/unit/planning/validation` | `tests/unit/codegen/assembly`, `tests/unit/codegen/execution` |
| Native ABI, statuses and namespaces | `AmaStatus`, byte views, six ABI records, typed size bridge and namespace context | C compiler checks exact signatures/target assumptions; trusted native code owns only native resources and bounded borrows | `tests/unit/native/abi`, `tests/unit/native/metadata/control`, `tests/unit/runtime/native-borrows`, `tests/unit/native/namespace`, `tests/unit/native/status-providers`, `tests/unit/native/toolchain/symbols` | `tests/integration/native/toolchain`, `tests/integration/parser/grammar` |
| Native model and generated interface | `NativeScaffold`, validated type/member descriptors | Separate model construction from emission; generated headers compile independently and preserve module qualification | `tests/unit/native/scaffold/declarations` | `tests/integration/scaffold` |
| Artifact ownership and publication | Scoped ownership model plus frontend-free artifact/IO publication interfaces | Separate preflight/staging/commit/rollback and descriptor lifetimes; never overwrite handwritten files implicitly | `tests/unit/native/scaffold/ownership`, `tests/unit/native/scaffold/filesystem`, `tests/unit/native/scaffold/publication` | `tests/integration/scaffold`, `tests/integration/execution` |
| Context identity and admission, `src/runtime/context/` | Context-owned world/program/pool/constant/error/control storage and checked host leases | Exact origin/staleness checks precede local storage; one executor per context; reverse successful-root cleanup | `tests/unit/runtime/contexts` | `tests/unit/runtime/host-lifecycle`; `tests/unit/codegen/contexts`, `tests/unit/codegen/parallel-contexts` |
| Stable type pools and checked local handles | `AmaPool`, `AmaHandle`, reserve/publish/invalidate/reclaim | Reserve final identity before initializers; growth preserves payload addresses; retirement/reclamation never revive identities | `tests/unit/runtime/pool`, `tests/unit/runtime/alignment` | `tests/unit/runtime/objects/lifecycle` |
| Ownership construction, cloning and replacement | `AmaWorld`, `AmaRecipe`, `ama_object_*` and checked native completion | Keep raw-success cleanup credits, initialized-prefix ledgers, subtree preflight and one injective clone map; separate preparation from destructive commit | `tests/unit/runtime/objects/construction`, `tests/unit/runtime/objects/lifecycle`, `tests/unit/runtime/objects/clone`, `tests/unit/runtime/objects/clone-preparation`, `tests/unit/runtime/objects/replacement`, `tests/unit/runtime/objects/access`, `tests/unit/runtime/protocols` | `tests/unit/runtime/transfers`, `tests/unit/codegen/execution` |
| Invocation and typed value transfer | Program selection, preparation, acquisition and return interfaces | Resolve once in specified order; preserve caller/callee namespaces, constructor choice, protections and refusal boundaries | `tests/unit/runtime/calls`, `tests/unit/runtime/call-preparation`, `tests/unit/runtime/value-selection`, `tests/unit/runtime/list-transfer`, `tests/unit/runtime/transfers`, `tests/unit/runtime/program` | `tests/unit/codegen/execution`, `tests/unit/codegen/continuations` |
| Membership, list methods and live traversal | `src/runtime/lists/`, `src/runtime/program/lists/` and resolved lexical cursor plans | Ordered stable occurrence identities, nonwrapping incarnations, live boundary corrections and per-method refusal/commit effects | `tests/unit/runtime/membership`, `tests/unit/runtime/traversal`, `tests/unit/runtime/traversal-mutation`, `tests/unit/runtime/traversal-origin`, `tests/unit/runtime/list-service`, insertion/deletion/ordering/transfer leaves | `tests/unit/codegen/traversal`, `tests/unit/codegen/list-insertion`, `tests/unit/codegen/list-deletion`, `tests/unit/codegen/list-ordering`, `tests/unit/codegen/snapshots` |
| Explicit control and execution diagnostics | Service metadata, return links, resume positions, source traces | Saved position differs from active caller link; yield releases acquisitions; failure reports before cleanup | `tests/unit/native/metadata/control`, `tests/unit/runtime/calls` | `tests/unit/codegen/continuations`, `tests/unit/codegen/execution` |
| C assembly and native-entry verification | Validated scaffold/plan descriptors plus output stream | One assembly per component; local emission operations take narrow models; context state is materialized separately from immutable code/type templates | `tests/unit/codegen/control`, `tests/unit/codegen/assembly`, `tests/unit/native/toolchain/symbols` | `tests/unit/codegen/execution`, `tests/integration/execution` |
| CLI and reusable compiler stages | `DriverOptions`, `CompilerFrontend`, command-specific orchestration | Recognition, storage, validation and execution are separate; exact options, explicit ownership and shared frontend; no duplicate compiler pipeline | `tests/unit/driver/request`, `tests/unit/driver/options`, `tests/unit/driver/run`, `tests/unit/compiler/check`, `tests/unit/compiler/status` | `tests/integration/launcher`, `tests/integration/check`, `tests/integration/scaffold`, `tests/integration/execution` |
| Standard scalar/refusal/failure library | Declared modules, typed native hooks and exact ordinary/standard root identities | Arbitrary-precision values, required integer size bridge, coherent failure description, no action enumeration ABI | `tests/unit/codegen/scalar-library`, `tests/unit/codegen/failure-values`, `tests/unit/codegen/library-compositions` | `tests/integration/standard-integer`, `tests/integration/failure-values`, `tests/integration/examples` |
| Compiled component models, exports and checked transport | `src/library/`, generated component adapters and `src/runtime/library/` | Separate admission/execution outcomes, authenticated component/context/instance identities, owned snapshots and pinned allocator/code lifetimes | `tests/unit/library/configuration`, `tests/unit/library/header`, `tests/unit/library/lifecycle`, `tests/unit/library/execution`, `tests/unit/library/transport`, `tests/unit/library/snapshots`, `tests/unit/library/nested` | `tests/integration/library-build`, `tests/integration/library-exports`, `tests/integration/c-library-examples` |
| Cooperative scheduling, task state and native events | Declared task/wait/queue state with native-only submission/registration ownership | Exact registration/run identities, bounded round work, unique FIFO readiness, explicit cancellation and late-result cleanup | `tests/unit/async/events`, `tests/unit/async/phase`, `tests/unit/async/generation`, `tests/unit/async/outcome`, `tests/unit/library/async` | `tests/unit/codegen/scheduler-rounds`, `tests/unit/codegen/event-bridge`, `tests/unit/codegen/task-cancellation`, `tests/integration/async-examples` |
| Parallel contexts, bounded native channels and typed messages | Independent context admission, `src/native/channels/` and explicit value codecs | No cross-context Ama handles/borrows, whole-message FIFO ownership, byte/count bounds, exact shutdown and executor transfer | `tests/unit/async/channels`, `tests/unit/async/messages`, `tests/unit/async/channel-waits`, `tests/unit/library/supervision` | `tests/unit/codegen/parallel-contexts`, `tests/unit/codegen/executor-transfer`, `tests/unit/codegen/channels`, `tests/unit/codegen/channel-waits`, `tests/unit/codegen/parallel-supervision` |
| Editor, debugger and relocation tools | `src/editor/`, `src/runtime/debugger/`, `src/tools/relocation/`, source/relocation driver commands and retained-assembly client | Shared grammar/resolution, exact identities and scope, fresh document authority, exclusive inspection, durable commit independent of output acknowledgement | `tests/unit/editor`, `tests/unit/runtime/debugger`, `tests/unit/tools/debugger`, `tests/unit/tools/relocation`, `tests/unit/driver/editor`, `tests/unit/driver/relocation` | `tests/unit/codegen/debugger`, `tests/integration/tools/source`, `tests/integration/tools/relocation`, `tests/integration/tools/debugger`, `tests/integration/tools/workflows` |
| Build, C harness and Doxygen | Recursive source discovery, command manifests, parent/leaf Makefiles, `make docs` | Explicit prerequisites and cleanup; every maintained code file has meaningful tests and generated documentation; no hidden generated-input analysis | `tests/integration/rebuild/tracking`, `tests/integration/rebuild/concurrent`, `tests/integration/harness/lifecycle`; `tests/unit/documentation` | Root `make test`, `make docs`, release acceptance |

All test homes in the table exist. Cohesive rows may name a tested parent subtree;
its explicit child lists lead to independently executable C/assert leaves. The
per-file [ready registry](documentation/ready.json) supplies exact named cases,
commands and standalone header checks, while the
[contract registry](contracts/README.md) supplies component domains, arguments,
failure partitions and assumptions. New test leaves need an explicit
`main`/`it_*`/`run` and actual parent registration.

## Responsibility dependencies

```text
original source bytes and complete identities
    -> layout and structural AST -> dependencies and typed bindings
    -> checked control/lifecycle IR and storage plans
    -> single project C assembly and verified native compilation
    -> context-local execution, lists, continuations and value ownership
    -> checked C components, asynchronous/parallel consumers and tools
```

The compiler has one owning implementation per boundary. Generated code consumes
resolved IR rather than guessing names or reparsing source; native/component
transports do not replace internal Ama label dispatch. Stable storage,
construction/clone maps, acquisition/refusal order and context isolation remain
explicit at every consumer.

The [current boundary contract](contracts/current-boundaries.md) states the
cross-component invariants. Individual feature documents and per-file evidence
record the actual tested domain; generated Doxygen descriptions establish
navigation and documented interfaces, while T117–T122 separately check complete
requirement and release acceptance. A discovered semantic ambiguity belongs in
`AMBIGUITIES.md`, never in a fallback grammar or an undocumented runtime rule.
