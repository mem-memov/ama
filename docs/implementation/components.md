# Implementation component map

This is the result of the first task in [TODO.md](../../TODO.md).
[TECHNICAL_SPECIFICATION.md](../../TECHNICAL_SPECIFICATION.md) remains the source
of truth. This map assigns implementation responsibilities; it does not change
language rules or certify the existing implementation.

## Evidence and replacement policy

The inventory below was obtained from source filenames and build configuration.
Existing compiler bodies, native implementations and test cases were not read or
analyzed. No existing algorithm, API or test is accepted as conforming merely
because a file exists with a relevant name.

Replace the old compiler and standard library implementations. Remove old test
cases without translating their assumptions into the new implementation. Preserve
the testing approach: recursive parent Makefiles, independent C leaf executables,
`assert`, descriptive `it_*` functions, explicit `run()`, and guarded parent
`common.c` fixtures. Preserve handwritten/generated output separation and cleanup
on success with artifacts retained on failure.

Do not create a second production pipeline or a compatibility backend. The [foundation reset](foundation.md) has established the new source/test tree
and removed the superseded files. The table below records their former locations.
A command whose implementation is not yet available must fail explicitly rather
than report successful compilation. A foundation build is not a working compiler.

## Component assignments

The existing locations identify removal scope, not APIs to retain. Destination
names organize the responsibilities required by specification section 12; create
subdirectories when their meaningful operations are implemented, not empty shells.

| Responsibility | Existing locations to replace | Destination | Contract/output |
| --- | --- | --- | --- |
| Command-line orchestration | `src/main.c` | `src/main.c`, `src/driver/` | Explicit commands, exit status and output ownership; delegate compilation/checking to shared services |
| Source storage and identities | `src/common/source.*`, `path.*`, `names.*`, `limits.h` | `src/source/`, with `text/`, `paths/`, `utf8/`, `identity/` children | Owned immutable bytes, stable spans, strict decoding, logical/physical mapping and collision-checked identity |
| Structured diagnostics | `src/common/diagnostic.*` | `src/diagnostics/`, with `render/` children | Structured causes/spans and deterministic rendering without losing source names |
| Layout and lexical reading | `src/lexer/` | `src/lexer/`, with `layout/`, `literals/` children | Structural lines/tokens and untouched literal payloads; no name-dependent interpretation |
| Syntax and AST | `src/parser/`, `src/ast/` | `src/parser/`, with `module/`, `body/calls/` children; `src/ast/` | Complete source constructs with spans and explicit ownership; no semantic resolution during parsing |
| Project dependency closure | `src/project/` | `src/project/dependencies/` | One registry per project, complete dependency closure, deterministic identity and requesting spans |
| Binding and type/lifecycle checking | `src/symbols/`, `src/semantic/` | `src/semantic/`, with `names/`, `types/`, `initialization/` children | Bound, validated typed values; module namespace, capability and initialization checks |
| Dependency relations | `src/graph/` and project/semantic consumers | Relation-specific children under `src/project/dependencies/` and `src/semantic/initialization/` | Exact traversal/order/cycle rules for each relation; no universal ban on module cycles |
| Control-flow lowering | No dedicated source subtree in the inventory | `src/lowering/`, with `calls/`, `control/` children | Explicit typed IR for calls, acquisition, result routing, failure edges and continuation positions |
| Storage planning | `src/codegen/type_plan.*`, `types.*` | `src/planning/`, with `layouts/`, `lifecycle/` children | Owned/interface/shared layouts, capabilities and service metadata, independent of C text |
| C assembly emission | Remaining `src/codegen/` files and project emission entry points | `src/codegen/`, with `types/`, `native/`, `execution/`, `diagnostics/` children | One dependency-correct project assembly, native entries, labels and source maps |
| Native ABI and toolchain checks | `src/runtime/value.*`, `record.*`, `call.*` and native codegen | `src/native/`, with `abi/`, `toolchain/` children | Fixed symbolic bindings, typed contracts, common status and explicit toolchain conformance checks |
| Runtime services | `src/runtime/` | `src/runtime/`, with `pools/`, `references/`, `lifecycle/`, `execution/`, `lists/`, `diagnostics/` children | Stable instance storage, checked handles, exact lifecycle and explicit execution transitions |
| Standard library | `stdlib/modules/sys/` and its build integration | `stdlib/modules/` with documented module/companion paths | Typed scalar/resource operations, constants, conversions and refusal reader; no special numeric core |
| Tests | Existing leaves and fixtures under `tests/unit/`, `tests/integration/` | New leaves under the same two roots | Independently runnable contract tests and real composition/generated-C tests |

All existing production files inventoried under `src/` belong to these replacement
scopes. Paths ending in `.*` cover their `.c` and `.h` files. There is no approved
production-code reuse in this inventory. The shared test/build mechanism is the
only selected reusable implementation approach; its files may be simplified or
rewritten without changing that approach.

## Build foundation findings

Build configuration observed before the reset:

- The root Makefile discovers `src/*.c` and `src/*/*.c` only. It cannot discover
  a file such as `src/source/utf8/decode.c`.
- Production objects already use compiler dependency files. Preserve that
  mechanism for actual included headers throughout the deeper source tree.
- `tests/leaf.mk` collects `src/*/*.h` with a shallow wildcard. Replace that
  dependency approximation with compiler-generated dependencies for actual test
  translation units and their included fixtures/headers.
- The root `all` and `test` targets require the old compiler and numeric library
  archive. Remove those implementation-specific couplings when resetting the
  source tree; test new components without requiring an unfinished compiler.
- Leaf `all` executes tests before recursive cleanup. Keep that ordering and
  retain outputs on failed execution. Keep `-UNDEBUG` so assertions remain active.

Build discovery must enumerate the complete finite source tree deterministically.
Incremental correctness must follow from declared/generated dependencies, not
filename-depth assumptions or unconditional rebuilds. Validate this with an
actual nested component, its assembled parent, and header-change rebuild checks
when implementing the foundation tasks. Do not use old tests as that evidence.

## Boundaries for the new implementation

Each component exposes only the types and operations its consumers need. Declare
allocation ownership, borrowed-view lifetimes and source-span lifetime explicitly.
Avoid a mutable global compiler context and a catch-all common helper collection.
Private helpers stay private; public headers must compile independently.

Consumers proceed through the following dependency order:

```text
source storage / identities / diagnostics
    -> lexical layout -> AST parsing
    -> dependency closure / binding / type and capability checks
    -> typed control-flow IR and storage/lifecycle plans
    -> one C assembly and toolchain verification
```

Native ABI constraints are validated independently before the frontend is
connected to native emission. Runtime services are tested independently before
execution integration. Shared constants have their own initialization relation;
module loading, eager ownership layout and dynamic acquisition are distinct
problems with distinct rejection rules.

For every substantive algorithm, record the domain, pre/postconditions, invariants,
failure effects, termination and complexity argument before integration. Derive
component tests from those laws and composite tests from the real handoffs.
The map itself is not a proof of any algorithm or a replacement for executable
acceptance evidence.

## Completion boundary

Completed here: structural inventory, replacement decisions, responsibility map,
and identified build/test foundation changes. No compiler algorithm was analyzed
or reused, no old test was analyzed, and no implementation test was claimed.

The subsequent foundation task removed those implementations and test cases and
introduced new driver components and tests; see [foundation.md](foundation.md).
Recursive discovery and build invalidation checks are recorded in
[incremental-build.md](incremental-build.md). Test-harness acceptance, including
output preservation, is the next task in TODO.
