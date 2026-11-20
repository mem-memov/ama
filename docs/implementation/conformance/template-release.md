# Clean template sprint release

## T197 contract and execution plan

Review input: `a2b866c`, after accepted current conformance review T196. This
record extends the existing [release workflow](release.md) with actual current
clean-build, full-test, example/toolchain and resource evidence. It does not
change the specification or substitute documentation counts for executed tests.

Rules: [TEST-05](../../../spec/implementation/tools-and-conformance.md#test-05),
[STACK-04](../../../spec/implementation/compiler.md#stack-04),
[COMP-05](../../../spec/implementation/compiler.md#comp-05), and
[ARCH-06](../../../spec/implementation/compiler.md#arch-06).

1. Capture exact maintained/specification inputs and tool versions. Run
   `make clean`, then build the compiler with strict O2/Werror/C11/NDEBUG flags.
2. Run the complete recursive test tree with those flags from that clean state;
   test assertions stay enabled by the existing `-UNDEBUG` leaf contract. Retain
   commands, live session/log paths, actual completion and unchanged input proof.
3. Run current standalone Ama and public C/async-host examples in fresh outputs;
   review finite nesting/growth/cache/native inlining, independent state, exact
   invalidation, resource refusal and publication evidence from actual full-tree
   consumers. Keep unsupported instrumentation/platform claims explicit.
4. Reconcile current release usage/results and run conformance/per-file/Doxygen
   gates for final task inputs. Archive and commit only collected acceptance.
   T198 remains the final generated documentation/reproducibility checkpoint.

Invariant: one job owns each generated destination. No source/spec/input changes
are made while a run claiming their stable snapshot is active. Failed probes are
not successes; if a source fix is required, collect the prior outcome, record the
reason, update its contract/tests and rerun the affected clean acceptance.

## Configuration

Strict compiler/test flags:
`-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG`.
The complete test command uses `-k -j4` to collect independent failures rather
than mask a later branch; a passing exit still requires every child success.
No semantic nesting/specialization/time limit is introduced for acceptance.

Current step: capture input/toolchain authority and run clean compiler build.

## Live clean-build checkpoint

Input authority is captured in `/tmp/ama-t197-inputs.json`: **3054** exact
maintained/specification/registry/matrix byte fingerprints. Toolchain versions
are in `/tmp/ama-t197-toolchain.log`. Session **66964** owns sequential
`make clean` and strict release compiler build, with separate logs and
`/tmp/ama-t197-{clean,compiler}-result.json` receipts only after completion and
unchanged-input comparison. Do not start the full test tree until this owner
finishes successfully. Current logs: `/tmp/ama-t197-clean.log` and
`/tmp/ama-t197-compiler.log`. Next action: collect this session and inspect any
actual diagnostic before choosing a follow-up.

## Clean compiler accepted; full test tree active

`make clean` and the strict O2/Werror/C11/NDEBUG compiler build both pass; their
result receipts confirm all **3054** captured inputs unchanged. The compiler
log contains 951 emitted command lines and no compiler error. No release-only
source workaround was required.

Session **47647** now owns the complete fresh recursive test run:
`make -k -j4 test C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11
-pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG'`. Live log:
`/tmp/ama-t197-full-test.log`; running/completed receipt:
`/tmp/ama-t197-full-test-result.json`. It records UTC start/end, actual exit,
elapsed time and changed-input list only after the parent completes.
Do not launch another job against any test generated directory. Next action:
collect this owner, inspect real failures if any, then run isolated fresh public
examples and final evidence/doc gates.

## Complete clean test tree accepted

The one complete fresh `make -k -j4 test` run passes with exit **0**, in
**3014.517 seconds** (50 minutes 15 seconds), from 2026-11-20 03:38:53 UTC to
04:29:07 UTC. `/tmp/ama-t197-full-test-result.json` confirms all **3054** captured
inputs unchanged and an empty changed-input list. No partial branch result is
being substituted for this parent outcome.

The full run includes every recursive unit/integration child, current conformance
checks, exact resource/failure models, native/toolchain checks, source tools,
static closure/guards/constants and actual generated/native/task/parallel
execution. Two failed C interface compilations printed by documentation and
conformance are deliberately rejected negative fixtures; both enclosing suites
and the entire parent pass. The last active scheduler scenario was `broken-wait`,
then its parent returned successfully. No assertion failure was reported.

Next step: isolated fresh examples, driven by `/tmp/ama-t197-examples.py` with
separate `/tmp/ama-t197-example-*.log` files and
`/tmp/ama-t197-examples-result.json`. It refuses to start without the successful
full-test receipt and checks unchanged inputs after every operation. Normal
async/template, public C library and native async-host consumers precede separate
ASan/UBSan public C/host builds (`detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`). No LSan/TSan claim follows from those runs.

## Public example checkpoint

The complete log contains **336** outer `./generated/test` invocations, matching
**336** registered C case-source files. The live registry names **1860** distinct
(source, case) pairs and **359** command strings; these are inventory counts, not
statement-coverage percentages. Actual resource output includes **529** process
interruption checkpoints, **1066** application I/O checkpoints, and independent
pending/terminal commit/rollback recovery allocation/I/O partitions. The full
parent exit, not searching for a diagnostic word, determines acceptance.

Example session **24040** has accepted normal async build/run (exact stdout
`completed\ncancelled\n`), template build/run (empty stdout/stderr), public
C-library consumers and async-host consumers. Each returns zero and retains the
input snapshot. The separate sanitized C-library and async-host examples remain
active/pending; collect `/tmp/ama-t197-examples-result.json` before claiming them.

## Public consumers accepted

All eight operations complete with exit zero and unchanged captured inputs:
normal async/template build/run, normal public C-library and async-host scripts,
and their separate ASan/UBSan variants. Exact stdout checks pass for the two
Ama programs. Receipt: `/tmp/ama-t197-examples-result.json`; complete individual
logs retain the actual commands and tool/native diagnostics. No example job
remains active. The public release guide now describes the accepted current
template implementation instead of presenting the obsolete pre-template claim.

The full run includes finite nesting, unequal-key growth/permutation rejection,
cache/constraint-order invariance, native inlining/toolchain verification and
independent storage/contexts. Component contracts retain exact finite-model
domains and general proofs; no semantic resource/time limit was added. Native
and source publication failures, exact input invalidation and measured cleanup
partitions execute through their existing actual recursive consumers.

Next: current conformance, complete metadata and actual Doxygen against the final
release guide/task evidence, then archive and commit T197. T198 retains the
final reproducible documentation delivery gate.

## T197 accepted result

Final conformance passes (`/tmp/ama-t197-conformance.log`) for 214 requirements,
150 acceptance rows and 56 groups. Complete per-file metadata passes with
3028 inputs and zero debt (`/tmp/ama-t197-metadata.log`). Actual Doxygen passes
(`/tmp/ama-t197-doxygen.log`): 2297 maintained pages, 8908 documented interfaces
and 452/452 canonical destinations. All required jobs are collected successes.

The accepted task is archived in DONE with this current release guide/evidence.
Regenerate documentation for these final archival bytes and compare the manifest
against staged inputs before committing T197. No production or specification
source changed; the full clean build/test and public consumer input proof
therefore remains exact. T198 owns final guide reconciliation, justified
fingerprint review and serial complete/docs/reproducible documentation delivery.
