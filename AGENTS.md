# Notes for coding agents

## Analysis scope

- Do **not** analyze binary files and build artifacts:
  - `ama` (compiler binary)
  - `build/` (object files, `build/tests/` intermediates)
  - any `*.o` or generated `.c` files under `build/`
- Compiler source lives under `src/`; standard library interfaces and C sources
  live under `stdlib/modules/`.
- Tests use recursive Makefiles under `tests/unit/` and `tests/integration/`.
  Each leaf has its own `main.c` with `it_*` functions and an explicit `run()`;
  guarded parent `common.c` files hold test fixtures. Assertions use `assert`.
- Integration module files live under each leaf's `project/`; expectations are
  `expected.txt` or `expected-error.txt` beside it.
- Native test interfaces and handwritten companion `.c` files live together
  under `project/`. Generated native headers belong under `generated/`.
- Do not analyze generated artifacts in any test's `generated/` directory.

## Specification authority

- The entire `spec/` directory is the source of truth for language semantics,
  product behavior, and implementation requirements. Start at `spec/README.md`;
  use `spec/requirements.md` to locate a stable requirement ID. The canonical
  grammar is `spec/grammar.md` and follows the language chapters. Do not recreate
  root-level specification or EBNF files as alternative copies or navigation stubs.
- Do not create, edit, move, or delete any file in `spec/` without prior explicit
  user authorization for that specification change. A TODO task, implementation
  request, refactoring request, or discovered contradiction does not provide this
  authorization. Never change requirements to fit the code. Existing authorization
  for a specific specification change is sufficient; do not ask again for it.
- No backward-compatibility documentation is required. Remove obsolete proposals
  and API descriptions instead of preserving alternative contracts.
- Follow ARCH-05/ARCH-06: use exact algorithms, forbid heuristics, and provide
  general mathematical/algebraic justification with explicit invariants and tests.
- Keep specification chapters readable: explain the model and motivation, show
  Ama examples with assumptions and expected effects, then state complete rules.
  Label informative explanation/examples; define each requirement once and link
  related chapters rather than duplicating semantics in the grammar.

## Resolve ambiguity before implementation

- If a requirement is ambiguous or contradictory, or required observable behavior
  is unspecified, stop the affected task before choosing an interpretation,
  heuristic, fallback, or workaround. Do not switch to another TODO task to avoid
  resolving the blocker. Read-only investigation needed to explain it may continue.
- Record the issue in the root [AMBIGUITIES.md](AMBIGUITIES.md) registry with a
  stable ID, `Open` status, current TODO task, exact specification links, a minimal
  example, the precise question, and the consequences of the possible answers.
  Ask the user for clarification and wait for an explicit answer. Silence and
  elapsed time are not decisions; do not mark or commit the task as completed.
- Record the user's answer and its source before resuming. An answer authorizes
  the stated behavior; it authorizes editing `spec/` only when it explicitly
  requests that specification change. If the answer requires a specification
  change without authorizing it, request that authorization before proceeding.
- Routine engineering choices with unambiguous specified behavior remain the
  agent's responsibility. Decide them using exact algorithms, explicit contracts,
  and evidence; do not turn ordinary design work into repeated permission requests.

## Style

- Follow `CONTRIBUTORS.md` for naming, formatting, component boundaries, and tests.
- Remove unused, obsolete and superseded implementation code together with tests
  whose only purpose is to exercise it. Retain no dormant alternatives or
  compatibility branches for abandoned language rules. Trace real consumers
  before deleting shared components; reusable foundations must have an explicit
  current implementation owner. Passing old tests does not establish conformity
  with the current specification. Replace obsolete cases with positive, negative,
  boundary and failure checks of the current rules and their real compositions.
- Use English text and descriptive identifiers. C indentation is four spaces;
  Ama fixtures retain semantic one-space nesting. Make recipes use `>` followed
  by four spaces.
- Decompose implementation as far as cohesive responsibilities allow, using nested
  directories, small single-operation functions, and narrow explicit interfaces.
  Keep orchestration separate from parsing, checks, state changes, and cleanup.
  Do not satisfy decomposition with empty folder levels, one-line public wrappers,
  unrestricted utility modules, or arbitrary file/function line-count targets.
- Every maintained code file needs a purpose comment and explicit test coverage.
  Follow the Doxygen and per-file coverage rules in `CONTRIBUTORS.md`, including
  headers, native companions, test support, scripts, and build infrastructure.
  New C source/header files and test C files use Doxygen `@file` documentation;
  document each nontrivial function/type/interface's purpose, contracts, ownership,
  side effects, and failure behavior. Explain why non-obvious code is needed.
- Compiler purpose comments and explanations of rule implementations must cite
  the relevant canonical `spec/` files and stable requirement IDs with clickable
  Markdown links. Resolve paths relative to the commented source file and include
  the rule anchor; see the source-location example in `CONTRIBUTORS.md`.
  Explain how the cited rule determines the operation or invariant; a bare ID
  is insufficient. Follow `CONTRIBUTORS.md` for
  coverage, source navigation, and generated Doxygen link validation.

## Target implementation boundaries

- Follow the compiler decomposition and component/composite testing requirements
  in `spec/implementation/compiler.md` and
  `spec/implementation/tools-and-conformance.md` (`ARCH-01` through `ARCH-06`,
  `TEST-06` through `TEST-08`).
- Ama modules translate through resolved IR into labels in one project C assembly;
  native entry bodies inline, while calls inside native C may use the stack.
- Instances execute directly in stable pool slots by default. Apply the compact
  representations and storage elimination permitted by RT-02 and the compiler's
  optimization contract only with a proof that observable identity, lifecycle,
  control, and native effects are preserved. Do not introduce working-state
  save/restore or implicit default instances. Persistent Ama references belong to
  declared Ama fields, never opaque native payloads.
- Compiled Ama libraries use the checked C boundary in `spec/implementation/c-library.md`.
  An Ama native wrapper may own an opaque foreign-context resource only under
  EMBED-07/EMBED-08; it must not hide ordinary Ama references or reenter a running
  context. C calls between isolated compiled components do not replace internal
  Ama label dispatch, and a child yield must return normally before an outer yield.
- Asynchrony is explicit and cooperative. `%` returns to the immediate caller;
  native callbacks do not retain borrowed Ama state or execute Ama. Parallelism
  uses isolated execution contexts with one executor each and explicit messages,
  under `spec/language/async-and-parallelism.md`; do not share mutable instances
  or checked Ama references between contexts.
- `name+/` is physical submodule storage, not part of the logical identity.
  Document implementation support only when backed by acceptance evidence.

## Workflow

- Follow the [development standard](docs/development/README.md) for every task.
  Read it before planning or implementation; use its linked task-record template
  proportionately. It defines the task/step lifecycle, evidence, safe execution,
  continuation checkpoints and acceptance procedure alongside these instructions.
- Add a stable TODO task for newly requested work before implementation. Plan
  prerequisites and small verifiable steps; record each meaningful step's purpose,
  actual findings/changes, checks, decisions and remaining work in TODO or a linked
  versioned task/component note. Temporary logs and chat are not the sole record.
  For long work, retain the current step, active jobs/logs and next action so a
  later turn can resume safely without repeating or falsely completing a run.
- Keep active tasks in [TODO.md](TODO.md) and completed records in
  [DONE.md](DONE.md). Allocate IDs across both files without reuse. After required
  acceptance passes, move the full task and evidence to DONE in the task's commit;
  preserve dependencies, decisions and results. Do not archive unfinished work
  or an unresolved documentation-debt owner. Consult DONE for completed prerequisites.
- When asked to implement or continue implementation, work from [TODO.md](TODO.md),
  respecting prerequisites and the stated acceptance criteria. Choose a ready task
  and finish its implementation, documentation, component/composition tests,
  and required checks before marking it complete.
  Continue through ready tasks within the authorized scope after each commit;
  completion of one substep or task does not cancel the rest of the request.
  Do not replace the roadmap with unrelated improvements or claim unsupported
  implementation coverage. Keep task status and evidence accurate.
- Commit after every completed task. This instruction is standing user
  authorization for those commits; no separate request is needed. Include the
  task's code, tests, documentation, and TODO status in the same commit. Use
  explicit staging paths, inspect the staged diff, and preserve unrelated user
  changes. Never mark an incomplete or blocked task complete merely to commit it.
  Do not combine several independently completed TODO tasks into a later batch.
- Build: `make` (compiler `./ama`); use `--emit ... --compile` for a verified project executable.
- Test: `make test`; individual suites: `make -C tests/unit/driver/request`.
- Test cleanup: `make test-clean`. Successful leaves clean automatically; failures
  preserve generated files for debugging.
