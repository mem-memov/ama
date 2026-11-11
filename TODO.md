# Ama implementation backlog

[TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md) is the source of truth;
[EBNF.md](EBNF.md) maintains its grammar. The agreed semantic decision packages
are closed. This file tracks implementation and acceptance, not alternative
proposals or claims of completed features. There is no compatibility obligation:
replace nonconforming mechanisms and remove obsolete documentation.

## Stage 0: Backend/native contract artifacts

- [ ] Implement the common code/message/length `AmaStatus` contract.
- [ ] Specify exact component algorithms, domains, invariants and failure effects;
  provide mathematical/algebraic justification under ARCH-05/ARCH-06 and derive
  acceptance tests from those laws under TEST-08.
- [ ] Generate checked native lifecycle/entry signatures for the six fixed `.abi`
  bindings: mandatory `+`/`-`, optional `|`/`=`/`?`/`[]`.
- [ ] Enforce native-entry inlining under the supported C toolchain while allowing
  ordinary internal C helpers, recursion, allocation, I/O and library calls.
- [ ] Implement the visible-Ama/opaque-native boundary: native resources in native
  payloads, persistent Ama ownership/references only in declared Ama fields.
- [ ] Build a narrow ABI harness covering successful construction, partial failure,
  destruction errors, continued cleanup and no retry/double destruction.

## Stage 1: Source, identity and syntax

- [ ] Enforce strict UTF-8 and pinned Unicode XID identifier tables, case-sensitive
  byte identity and unambiguous filesystem mapping.
- [ ] Resolve extensionless modules and sibling `name+/` directories using logical
  identities; derive companions/debug paths from actual physical filenames.
- [ ] Apply the one base-name namespace across constants, parameters, results and
  all persistent declarations, without branch/loop shadowing.
- [ ] Implement section parsing to first optional `@=`, then `@<`/`@>`, then `@!` or
  `@C`; recognize complete constants-only files and inactive unfinished drafts.
- [ ] Parse/check all owned/reference/list forms, `|` payloads, assignments,
  `!` entry, output-only `%` resume, `%` yield and `%source` cloning.
- [ ] Support empty positional `<`/`>` placeholders with independent arity counters;
  reject named mixing, empty `>>`, and all inputs on resumption.
- [ ] Implement qualified constant sources and static path-plus-`>>` reads, retaining
  instance-result read semantics and the prohibition on path execution/resumption.
- [ ] Add precise source-located negative and boundary tests for the accepted grammar.

## Stage 2: Dependency closure, semantic IR and decomposition

- [ ] Build one module registry/loader for transitive project/library dependencies,
  parse required modules once and reject missing/draft/duplicate identities.
- [ ] Distinguish load cycles, eager ownership layout cycles, constant initialization
  dependencies and dynamic busy-instance reentry.
- [ ] Resolve types and native capabilities into explicit control-flow/lifecycle IR
  and storage plans; never use emitted C strings as the semantic representation.
- [ ] Initialize immutable copyable constants before the root, with visible dependency
  checks, no hidden native initialization dependencies and reverse shared cleanup.
- [ ] Implement typed library enumerations and direct qualified action/outcome values.
- [ ] Organize every major compiler component into the nested responsibility tree
  in ARCH-01 through ARCH-04; keep orchestration/functions short and contexts narrow.
- [ ] Update compiler source discovery and test dependency tracking for deeper source
  directories; current shallow Make wildcards are not the target tree implementation.
- [ ] Add independently runnable tests for meaningful subcomponents and assembled
  parents, preserving ownership/span/type handoffs and failure behavior.

## Stage 3: In-place stackless execution

- [ ] Allocate stable block pools for executable module types, one explicit root,
  and no implicit callable object per module. Constants-only namespaces need no pool.
- [ ] Implement typed slot/generation handles, all-zero empty references, retirement
  on generation exhaustion, free-slot reuse and safe trailing-block reclamation.
- [ ] Execute directly over selected instance fields; no global working mirrors or
  caller/callee payload transfer around invocation.
- [ ] Emit ordinary module bodies once as labels in one execution function, with
  instance-owned caller/return metadata and direct/switch label dispatch.
- [ ] Preserve interface defaults/retention, permanent state, early return, protected
  result transfer, same-type nesting on different instances and busy-instance refusal.
- [ ] Build active operation/call-site maps and diagnostics before cleanup, including
  failure without allocating a complete trace buffer.
- [ ] Save acquisition refusal in caller context and expose a normal library reader;
  successful/nested calls must not overwrite the wrong context.
- [ ] Implement completed/yielded/failed host outcomes; root yield needs explicit host
  handling, not an implicit resume loop. Failed execution is not resumable.

## Stage 4: Native and lifecycle integration

- [ ] Include dependencies/types/constants/native bodies in one generated project C
  assembly; compile the launcher translation unit rather than separately calling
  Ama module C translation units.
- [ ] Integrate the fixed fresh/constructing/live/finalized protocol: failed constructors
  clean themselves, live children unwind in reverse, native destroy precedes children.
- [ ] Prepare copies before replacement commit. Preserve destinations on preparation
  failure; treat destructive-commit error as terminal with continued cleanup.
- [ ] Derive copy capability, reject unsupported owned cloning and allow reference
  copying regardless of target clone support.
- [ ] Clone ownership trees with internal reference remapping; preserve external
  targets, destination identity and whole-owning-list clone boundaries.
- [ ] Enforce self-assignment/no-op, `%` self-copy prohibition, ancestry overlap refusal
  and whole-source-tree availability checks before mutation.
- [ ] Verify generated C signatures, lifetimes, native helpers and C-entry inlining
  with positive/negative tests and source mapping checks.

## Stage 5: Suspension and continuation copies

- [ ] Save explicit branch/loop control positions at `%`; distinguish available yield
  from a busy caller waiting for a nested invocation.
- [ ] Resume without input bindings/reset; `!` always starts at entry and discards
  the target's old position while preserving permanent data.
- [ ] Clear positions throughout ordinary clones; preserve positions throughout `%`
  clones without caller chains, busy state or refusal-history metadata.
- [ ] Refuse either copy form if an included owned descendant is running/waiting;
  external reference targets do not expand the copy boundary.
- [ ] Test multiple yields, mixed yielded/completed children, explicit root resumes,
  clone failures and no preserved native C activation.

## Stage 6: Lists and library integration

- [ ] Implement contiguous ordered handle arrays with length/capacity over stable
  element pools, geometric growth, zero-based bounds and order-preserving removal.
- [ ] Implement the fixed four-input/three-output list interface and six actions,
  with skipped positional inputs/results and default Inspect.
- [ ] Support owning default construction/ordinary clone and reference-list handle
  storage, including empty references, without ownership transfer.
- [ ] Preflight Clear and other mutations; ordinary refusals preserve structure,
  while technical/destruction failures follow the common fatal cleanup contract.
- [ ] Keep retained length accurate; provide traversal through existing loop/index
  contracts and optional queue specialization in libraries.
- [ ] Publish scalar/action/outcome/error reader interfaces and library paths without
  inventing new core operators, universal integers or special English argument names.

## Stage 7: Acceptance and implementation completion

- [ ] Complete the technical specification's acceptance matrix at component,
  composition and generated-C integration levels.
- [ ] Cover transitive Ama/native dependencies, invalidation, repeated logical imports,
  Unicode paths, collisions, pool growth/reclamation, and constructor/clone failures.
- [ ] Run required build/tests, relevant ASan/UBSan and supported-toolchain checks;
  record actual acceptance evidence.
- [ ] Replace nonconforming backends and fixtures with implementations and tests
  of the accepted contracts; do not preserve obsolete behavior for compatibility.
- [ ] Update documentation and conformance evidence together;
  do not advertise unfinished target contracts as implemented features.

## Build and verification commands

```sh
make
make -C tests/unit/lexer/tokens
make -C tests/unit/runtime
make -C tests/unit/codegen/types
make test
make test-clean
```

Use the smallest relevant suites while implementing, then required broader checks.
Every successful test leaf cleans generated outputs; failure preserves artifacts.
Documentation-only maintenance needs consistency/link checks, not a fabricated
report of compiler/runtime tests. See [CONTRIBUTORS.md](CONTRIBUTORS.md) for the
harness and contribution rules.
