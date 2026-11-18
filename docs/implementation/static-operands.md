# Live static-expression operand preparation

## Task and boundary

[T214](../../DONE.md#t214) starts at `3289336` and the
[T203 input fingerprints](audit/static-alignment-inputs.json). The user's
2026-11-19 instruction explicitly requires deleting unused/obsolete code and
its sole-purpose tests. The audit found no production caller of the abandoned
`static_result_pattern_apply` API. Its six-case leaf is not a current consumer.

Remove that operation, its cached application field/public declarations and
its leaf. Keep only operand preparation consumed by actual expression uses;
make the cache an internal operation with a narrow immutable operand result,
not a new public interface-preview service. Adopt only the cohesive existing
result-expression extraction needed for that live operation; leave lexical,
typed-scope, congruence and other T182 changes outside this task's commit.

Authority: [TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11) and
[ARCH-03](../../spec/implementation/compiler.md#arch-03). No specification edits
or whole-program template acceptance are part of this cleanup.

## Contract and argument

Input is a registered original StaticResultSource owned by the supplied result
plan. It already binds one exact source expression and lexical environment.
The result is an immutable prepared StaticTermApplication borrowed from longer
lived result/binding/term owners. Only source input terms are prepared. There
is no equation solving, definition/output evaluation, creation entry, readiness
proof, runtime state or executable admission in this operation.

Cache keys are exact registered source identities, which include expression
and environment. Equal keys share immutable preparation; distinct environments
cannot share preparation by name or origin alone. Each actual expression use
still registers its own lexical child uses and later runs all required creation,
definition and output checks. Sharing source terms therefore cannot merge
incoming provenance or create a cache-based validation shortcut.

Separate operand collection from cache validation/publication. Prepare all
operand records before publishing a cache entry/count or caller output. Failed
preparation leaves the caller output untouched; partial records remain owned
and safely destructible with the sticky first error. Count/size overflow refuses
before publication. Do not promise rollback of other owner arenas.

For a source with N written local requests, collection visits each request once;
child sources are registered, not recursively prepared. Membership/cache scans
visit finite retained lists. The cache therefore adds O(S + C) lookup time for S
registered sources and C cached sources, plus existing binding/term construction
cost and O(N) operand storage for a new key. Each successful key publishes once.
Correctness follows by induction over its request prefix: every explicit input
retains exactly its original lexical child value, and no other operand is added.
The complete cache entry denotes that same list on every later lookup. Actual
use validation remains a separate operation, so no readiness conclusion follows.

## Steps and planned checks

| Step | Expected observation | State |
|---|---|---|
| Remove dormant preview | No source definition/declaration/call, cached application state or sole-purpose leaf remains | Complete; four pre-existing untracked files removed |
| Narrow the live preparation | Private operand collection/cache with purpose and ownership comments; actual expression consumer retains its checks | Complete; collection, cache and sibling contracts are separate |
| Cover real behavior | Current ordinary-source cases cover same-key sharing, distinct environments, no new creation/equations, overflow and all measured refusals | Initial eight-case prepare leaf exits 0 |
| Compose and deliver | Results/publication/constants strict and sanitizer parents, root build, per-file mapping and isolated documentation delivery | Behavior and corrected Doxygen pass; final record refresh and staged-byte agreement are commit gates |

Before editing, T203's actual existing strict leaves and warning reproduction
are recorded, not promoted to new acceptance. Run strict component parents
serially within each shared output tree; independent parents may use separate
leaf outputs. Relevant sanitizer flags are C11/O1 with address,undefined,
frame pointers and non-PIE, with `ASAN_OPTIONS=detect_leaks=0` under the already
recorded container restriction; counted allocation/teardown remains independent
ownership evidence. The existing T182 O2 warning remains owned there.

## Step results and exact adoption

Removed the original untracked `results/pattern/{apply,prepare}.c` and its
`tests/unit/semantic/static/results/pattern/{main.c,Makefile}` leaf. Only the
unused application service/state disappears; live preparation is implemented
under `results/operands/{collect,prepare}.c` with a sibling private header.
The public plan has only an opaque cache pointer/count. No public operand or
preview application API remains, and the source cache carries no application
record. Its actual expression consumer obtains immutable operands through the
internal header and retains each actual child/creation/definition context.

The task deliberately adopts only the existing result-expression extraction:
`expression/{application,apply,prepare}.c`, result `internal.h`, `model.h` and
`plan.h`, plus the new private operand component. The extracted application
kernel is called by the actual expression operation and retains independent
nominal-origin/creation checking. Other T182 statement catalogs, lexical/typed
connections, runtime holes and congruence code stay outside this commit. The
delivery registry is merged from HEAD with only these task-owned file/test rows;
unrelated local evidence is preserved separately, never blindly staged.

The new support cases use a concrete ordinary module with integer fields and a
self-reference. Actual guarded same-Type edges establish two distinct incoming
contexts with one exact nominal environment. Their operands share; the generic
environment does not share with them. A refused-allocation cache hit still
succeeds without allocation and changes no path/equation count or readiness.
Other cases reject copied/null source identities, null receivers and count
overflow without publication, then fail every observed live-preparation
allocation with counted zero-leak teardown. Initial prepare command
`make -C tests/unit/semantic/static/results/prepare -j4` exits 0, eight cases;
session 83050 is collected, log `/tmp/ama-t214-prepare.log`.

Next: strict and sanitized real results/publication/constants parents and root
build in a clean `3289336` archive plus only the task bundle. This proves the
adopted operations do not silently need the remaining working-tree T182 APIs.
Keep T203's original hashes as historical inputs, not falsely current hashes
after these intentional removals/adoptions. No specification change is needed.

## Verification results

The isolated task owner is `/tmp/ama-t214-check-fdpvoqwq`, created from `3289336`
plus only the nine result source/header inputs, two prepare-test inputs and
task-owned documentation/registry rows. The other T182 code is absent. Root
`make -j4` completed exit 0 (session 84298, `/tmp/ama-t214-build.log`). All three
strict parent commands completed exit 0. They use `-O0 -g -Wall -Wextra -Werror
-std=c11 -pedantic -D_POSIX_C_SOURCE=200809L` with assertions enabled.

Sanitized parents also complete exit 0, each after its own strict parent in the
same independent output tree: results session 34186, publication 53182, constants
84771 are collected. Logs are `/tmp/ama-t214-{results,publication,constants}-sanitized.log`;
per-parent command/exit receipts are `/tmp/ama-t214-<parent>-results.json`.
Sanitizer C flags are `-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer
-fno-pie`; link flags are `-fsanitize=address,undefined -no-pie` and
`ASAN_OPTIONS=detect_leaks=0`. Collect each completion before another job takes
its output tree. No executable C tokens change after the accepted strict runs;
the documentation-only header correction below is checked separately.

| Parent in the isolated task snapshot | Explicit cases per configuration | Strict | ASan/UBSan |
|---|---|---|---|
| `make -C tests/unit/semantic/static/results -j4` | 35 across source/context/cache/prepare/evaluate | Exit 0 | Exit 0 |
| `make -C tests/unit/semantic/static/publication -j4` | 29 across values/recursion/failures/contexts/closure | Exit 0 | Exit 0 |
| `make -C tests/unit/semantic/static/constants -j4` | 35 across selection/read/qualified/initializer/failures/declaration/catalog | Exit 0 | Exit 0 |

All 99 explicit cases run in each configuration; measured refusal loops and
independent finite models retain their additional internal assertions. No ASan,
UBSan or strict warning diagnostic is observed. The isolated root build has no
warning: unfinished typed-scope code is deliberately absent, so this is not a
claim that T182's separate working-tree warning has been fixed.

Source/test search confirms no `StaticResultPattern`, `static_result_pattern`,
`first_pattern` or `pattern_count` remains in the result component. The obsolete
leaf is removed from recursive routing and all its per-file evidence references
are removed. New private headers and live source/test files have named cases
and standalone-header configuration, to be checked by final documentation gates.
### Per-file evidence

The precise registry rows are in `docs/implementation/documentation/ready.json`.
They map each changed source/header/test to actual named cases and commands,
not merely a parent-pass label:

| Maintained file boundary | Independent behavior and real consumer |
|---|---|
| `results/operands/collect.c` | Existing nested/local-source and every-allocation preparation cases, plus the new current-source zero-input/refusal case, through `static_result_prepare` |
| `results/operands/prepare.c` | `it_shares_only_same_source_environment_operands_across_actual_contexts`, `it_refuses_foreign_operand_sources_receivers_and_cache_count_overflow` and every measured preparation refusal |
| `results/operands/internal.h` | Standalone first-include C11/Werror check and the collection/cache consumer cases |
| `results/expression/{prepare,application,apply}.c` | Source preparation plus `it_refines_actual_origins_without_repeating_unchanged_source_work` and `it_recovers_a_constructor_from_an_independent_whole_expression_type`, composed with actual guard/evaluation/publication cases |
| `results/{model,plan,internal}.h` | Standalone first-include checks and actual source/context/evaluation/closure plus new opaque-cache consumer cases |
| `tests/unit/semantic/static/results/prepare/{main.c,support/operands.c}` | Explicit eight-case run; three new current ordinary-source cases compile/run with real shared fixtures and counted teardown |
| Retained recursive Makefiles | Actual results parent dispatches only five live leaves; prepare owns the included support through tracked compiler dependencies |

Old preview files and sole-purpose test rows are absent. Other T182 registry
rows remain in the local working file; the commit uses only the explicit
task-owned merge against `3289336`. The removal/adoption does not erase the
earlier failed runs or claim the new static grammar is implemented.

## Final delivery checkpoint

Behavioral acceptance is complete. No source or test jobs remain active. Finish
task archival and links, then refresh only narrative/queue/registry inputs in the
same task-only snapshot; verify its tested source bytes against the working task
files before `make docs`. Doxygen/coverage/header/interface/link checks and exact
staged-snapshot agreement must pass before the separate T214 commit. The generated
reference stays unversioned and its actual result/tool versions accompany that
commit. Continue with T206 immediately afterwards; no spec clarification is open.

The first final `make docs` exits 2 (`/tmp/ama-t214-docs.log`, session 23299).
Coverage passes 2695 inputs with zero debts, but the actual HTML origin check
finds no ARCH-03 link on `operands/internal.h`'s file page. HTML inspection shows
TPL-08/TPL-11 there; ARCH-03 appeared only in the struct description rendered on
its separate class page. Add the actual complete-publication invariant and its
ARCH-03 link to the header's file-purpose explanation too. This corrects source
navigation and generated placement without weakening the documentation gate.
Only that comment changes; compare the compiler-preprocessed old/new header
bytes before regenerating on the updated final snapshot. Preserve this failed
run separately from the subsequent generation result.

The corrected generation exits 0 (`/tmp/ama-t214-docs-rerun.log`, session 91032):
2695 ready inputs, zero debts, 2037 maintained file pages, 7927 documented
interfaces, 431/431 destinations and 6461 HTML pages. Tools are Doxygen 1.11.0,
Python 3.14.4 and markdown-it-py 3.0.0. Old/new header preprocessing with
`gcc -std=c11 -D_POSIX_C_SOURCE=200809L -E -P -Isrc
-Isrc/semantic/static/results/operands -x c -` has exactly equal output bytes,
SHA-256 `fd423e0ef75f6f17febf32e43557770432f36acad3676c4fc5416631057a743c`.
This is a compiler check of the comment-only correction, not a weakened source
hash comparison. The final DONE explanation and this collected outcome are
refreshed before one final generation on the delivery records; no executable
changes or new behavioral concerns follow the accepted runs.
