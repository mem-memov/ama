# Independent project commands

## T233 scope and prerequisites

T233 follows accepted T232, T221 and T226 (`71af328`).
[PROJECT-01](../../spec/implementation/project-tools.md#project-01) separates
command ownership from compilation; [PROJECT-02](../../spec/implementation/project-tools.md#project-02)
defines initialization. [PROJECT-03](../../spec/implementation/project-tools.md#project-03)
defines isolated test orchestration. No specification edits are part of this task.

The separate executable is built from `tools/project/`, outside `src/` and the
compiler dispatcher. Initialization never invokes the compiler, a shell, network
transport or Git. Filesystem paths remain byte strings, distinct from Ama names.

## Steps and acceptance plan

1. Parse the exact `init DESTINATION` request into a borrowed immutable view;
   reject missing, excess and unknown arguments before effects. Test this child
   independently and through the command adapter.
2. Describe the fixed three-file scaffold and useful local guidance. Traverse
   destination parents through directory descriptors without following symlinks;
   require an existing parent and a new or empty ordinary destination.
3. Prepare all files in a private sibling directory, then publish by one atomic
   directory rename. Keep a nonblocking advisory parent-directory lease across
   preflight/preparation/publication so cooperating invocations cannot overlap.
   Kernel rename also refuses replacing nonempty destinations. Cleanup removes
   only known operation-owned entries; report retained staging on failure.
4. Exercise actual new/empty/conflicting destinations, symlink components,
   controlled concurrent claims and every filesystem preparation failure.
   Verify exact output membership, useful guidance and no unrelated mutation.
5. Add independent test-job planning and execution with caller-owned output
   destinations and explicit adapter ownership. Reject duplicate/conflicting
   jobs before invocation. Real compiler executions demonstrate separate test
   programs; T242 later supplies the accepted CMake adapter without a dependency
   cycle. Record exact job inputs and outcomes.
6. Register per-file named tests, exercise command/composition and sanitizers,
   generate Doxygen, archive T233 and commit.

## Command contract

The parser owns no storage and has no effects. A successful request borrows argv
through command completion. Only the exact three-argument initialization form
(including argv[0]) is admitted. An empty destination is invalid. The command
adapter reports usage failure without entering filesystem code.

## Filesystem invariants and proof obligations

The scaffold contains exactly root README.md, root AGENTS.md and packages/README.md.
No source/package identity is invented. Private staging is a sibling under the
retained parent descriptor, avoiding cross-device publication. Parent traversal
rejects symbolic links component by component. The descriptor and exclusive
lease remain live through cleanup. A failed preparation cannot modify an existing
destination; publication has one linearization point, the rename. The kernel
refuses file/symlink/nonempty-directory targets. Cooperating initialization of
the same parent is serialized by the lease; arbitrary uncooperative mutation is
checked again at publication and cannot authorize merging or overwriting files.

Traversal consumes one path component per step; the scaffold and cleanup have
fixed finite membership. Space is O(authored and resolved path bytes). Parent traversal scans those
bytes linearly; preparation writes the fixed guidance bytes and performs one
exclusive-name probe per existing staging-name collision before its successful
probe. Filesystem enumeration/resolution costs remain explicit host I/O. Failure injection is an explicit I/O
admission interface, never a substitute for actual system-call result checks.

## Continuation

Current step: command recognition and fixed scaffold interface. No acceptance
checks have run yet for T233. Next: implement and test these small components,
then descriptor-backed initialization and isolated test orchestration.

### Step 1 observations

The independent request parser and immutable three-file guidance plan are in
`tools/project/command/` and `tools/project/init/content.*`.
`make -C tests/unit/tools/project/command` exited zero; missing/excess/unknown
arguments, null/empty inputs, output preservation, literal shell metacharacters
and exact guidance membership/local links are checked. Guidance uses the actual
current compiler `--emit ROOT MODULE --out OUTPUT --sdk SDK` request shape, with
an explicit Standard root; it does not invent a project entry or promise future
project commands already work. Next: descriptor-backed scaffold transaction.

### Initialization composition results

`make ama-project` and `make -C tests/unit/tools/project/initialization` both
exited zero (`/tmp/ama-t233-build.log`, `/tmp/ama-t233-initialization.log`).
The separate executable links no compiler dispatcher. Real transactions cover
new/existing-empty destinations, files/nonempty directories, final/intermediate
symlinks, cancellation at every observed boundary, every counted allocation
failure, retained-stage reporting, a user file introduced before publication,
and a pipe-synchronized competing initializer. A real separately executed
`ama-project init` creates the exact three-file plan. No shell, compiler, network
or Git operation belongs to initialization. The generated command example was
corrected against the existing driver grammar before these accepted checks.

Current step: isolated test orchestration. Initialization still needs broader
path/OS-failure/guidance command cases, sanitizer checks and the final per-file
registry/documentation gate. T233 is not complete or committed.

### Independent test-job and process contract

The testing API borrows prepared named jobs, exact canonical selected identities,
arguments and provenance. It reuses the compiler's pure canonical identity
recognizer, not its dispatcher. Distinct names may select the same module with
different arguments. Repeated job names are rejected before output creation.
All jobs are validated first in O(B + N²L) time, where B is the total input
size, N the job count and L the longest job name; exact name comparisons
account for the L factor.
Physical output-parent resolution and exact component-prefix checks exclude
packages/ and dependencies/, including aliases to those directories.

A fresh exclusive experiment directory contains numbered job directories. Each
job records its request before adapter invocation and its observed stage, exit
and signal afterward. Request fields use decimal byte lengths followed by exact
bytes and a comma/newline terminator, preserving empty arguments and embedded
newlines without shell escaping. Records include a version, job name, canonical
module identity, prepared provenance and the complete argument sequence.
The caller/accepted build adapter supplies truthful resolved closure membership,
source/configuration hashes and tool editions; this orchestrator neither guesses
them nor substitutes a module label for an actual build manifest. T242 composes
that native/build record with these requests.

Execution is sequential and synchronous. A callback receives only its job and
new output directory; it must create its own fresh execution context/process.
Normal compilation/program failures are recorded and independent jobs continue.
Infrastructure or report failure stops collection, preserves completed results
and retains the partial experiment directory for diagnosis. Invalid plans leave
caller outcomes untouched. No hidden test registration or persistent shared Ama
state exists. Space owned by the orchestrator is O(output path bytes), beyond
caller-owned inputs/results and adapter resources. It retains only one job path.

The process adapter passes argv verbatim, redirects explicit streams and waits
through interruption. A close-on-exec error pipe distinguishes launch failure
from a legitimate exit 127; the pipe contains at most one integer, so waiting
before reading cannot exhaust its capacity. Nonzero exit and signal termination
are observations, not successful program execution. No Ama state crosses this
boundary. Test output directories persist under caller ownership on success or
partial failure.

### Composed checks and corrections so far

`make -C tests/unit/tools/project/experiments` exited zero for exact records,
separate processes, ordinary failure aggregation, invalid selections, namespace
aliases, every counted orchestrator allocation refusal, partial evidence, exec
errors, normal exit 127 and signal termination. Common fixtures were separated
from initialization-only control code to remove unused helper warnings.

The first actual Ama compilation failed because its package fixture omitted the
required src directory. Both test modules now actually call a shared src/helper,
so the complete source/test structure is exercised rather than adding dormant
fixture code. The next composition compiled and executed both programs but its
assertion incorrectly expected the compiler's normal emitted-assembly message to
be absent. Compile and execution logs are now separate; the program output stays
independently checked. Those failed commands are not acceptance passes.

The project executable now owns a separate recursive Makefile and configuration
manifests, including its exact link input list. Only the pure source-name scanner
is shared. This avoids inheriting the compiler's binary/link configuration.
Current build: `make -j2`, log `/tmp/ama-t233-independent-build.log`, session 72303.
Next: collect it, rerun the full project subtree and real compiled leaf, then
finish incremental build checks, per-file registry, sanitizers and Doxygen.

### Accepted behavior and build checks before documentation

`make -j2` exited zero (session 72303). The full project subtree then passed
(session 54268), followed by another full pass after dot-endpoint support and
truthful source/configuration hashes were added (session 6384,
`/tmp/ama-t233-project-final.log`). Relative/absolute paths, literal shell
metacharacters, existing empty terminal-dot destinations, invalid endpoints,
usage diagnostics, actual file-open conflicts and unplanned retained staging
contents now have explicit cases. Terminal dot/parent endpoints first traverse
all authored components without following links, then verify the physical
endpoint against the retained descriptor before one final ordinary pass.
This normalization has at most two passes and does not admit symlink shortcuts.

`make -C tests/unit/tools/project/build` passed (session 49430,
`/tmp/ama-t233-build-contract.log`). It builds the real independent tool into
private output/object locations, verifies a repeated build preserves the binary
timestamp, removes mandatory shared link inputs and observes real link refusal,
restores them, injects an invalid compiler flag and observes real compile refusal,
then restores the profile and actually initializes a project with the binary.
This covers configuration and exact link-membership invalidation without editing
maintained sources or racing the normal build tree. Intentional failures are
asserted inside a successful harness, distinct from earlier failed commands.

Remaining acceptance: final current per-file registry, a fresh complete project
parent including the new build leaf, ASan/UBSan of initialization/orchestration,
Doxygen and final archival. No new project test CLI or accepted CMake build/run
command is claimed; the exposed synchronous test API is the T233 foundation.

### Sanitizer and final-check checkpoint

ASan/UBSan initialization (52819) and experiment/process tests both exited zero:
`/tmp/ama-t233-sanitized-init.log` and
`/tmp/ama-t233-sanitized-experiments.log`. Commands set
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`, compile flags
`-O0 -g -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` and
`LINKER_FLAGS=-no-pie`. LeakSanitizer was explicitly disabled; counted allocation
refusals and live-allocation assertions are separate ownership evidence. The
experiment cases additionally check signal-result aggregation, exact stdout
argument transport and a packages-prefixed sibling that remains a valid output
parent. Path/index buffers now derive variable numeric suffix capacity from
the corresponding integer bit widths, avoiding arbitrary name/index limits.
Final normal/sanitized reruns and documentation must cover those final bytes.

Final review additionally required explicit additional prepared source roots in
the test request: Standard-library or extra compiler roots can live outside the
project's packages/dependencies. They are also physically excluded from output
placement, using the complete root set supplied by the read-only preparation
consumer. A dedicated case checks this exclusion before adapter invocation.
The runner does not guess Standard roots from module spelling or environment.
This is an implementation-contract completion of PROJECT-03, not a language
change. Final checks must include this last boundary.

### Final behavior acceptance before documentation

The complete five-leaf project subtree passed at
`/tmp/ama-t233-namespace-final.log` (10054): strict commands, initialization,
process/experiment boundaries, two real Ama compilations/executions and actual
incremental build failure/recovery. There were no compiler warnings or failed
assertions. The final experiment ASan/UBSan command with the flags above passed
at `/tmp/ama-t233-sanitized-namespace-final.log`, including additional source-root
exclusion. Initialization ASan/UBSan already passed with the final behavior at
`/tmp/ama-t233-sanitized-final-init.log` (98900); later initialization edits are
purpose comments only.

All maintained new inputs are mapped to named cases in
`documentation/ready.json`; headers include standalone compilation commands.
Current step: freeze maintained inputs and run `make docs`, log
`/tmp/ama-t233-documentation.log`, to check actual tests/header contracts and
generated Doxygen links. Do not treat its launch as completion. After a pass,
archive T233, regenerate Doxygen for Markdown-only archival, verify accepted
non-Markdown fingerprints and staged bytes, and commit before T234.

The first complete documentation command passed all 3312 per-file inputs and
zero-debt checks, but Doxygen rejected undocumented extracted interfaces. The
missing items were purpose comments on the initializer status enum, outcome
record and several test/harness functions. Their purpose contracts are now
explicit; no executable behavior or interface layout changed. This failed
command is preserved at `/tmp/ama-t233-documentation.log` (3739). Repeat the
complete gate at `/tmp/ama-t233-documentation-final.log`; do not count the earlier
Doxygen failure as documentation acceptance.

The repeated complete gate passed (48512): 3312 ready inputs, zero debts,
2496 maintained file pages, 9615 documented interfaces and 491 destinations.
Final inventory review found that a maintained test leaf named `build` was
excluded both by the artifact filter and Git's build-directory ignore rule. Its
two files had executed, but were not yet deliverable maintained inputs. Rename
the leaf to `building`, register both files explicitly and update all live case
commands. The command recorded earlier remains a historical run at its former
path. Run the renamed leaf and repeat the complete documentation gate before
archival; the prior page count must not be reported as covering that leaf.

`make -C tests/unit/tools/project/building` exited zero at the maintained path
(`/tmp/ama-t233-building-final.log`, 74061); Git no longer ignores the inputs.
The renamed test still proves real link/compile refusal and complete recovery.
The final complete gate is `make docs`, log
`/tmp/ama-t233-documentation-maintained.log`. Inputs are frozen for that check.

The maintained-path gate checked all 3314 ready inputs with zero debts, then
Doxygen exposed the newly visible build case's missing purpose comment. Add
that comment only; the named case, command, interface and executable statements are
unchanged. The failed gate is `/tmp/ama-t233-documentation-maintained.log`
(72662). Reuse its successful inventory/header checks for those unchanged
contracts and rerun the supported Doxygen builder at
`/tmp/ama-t233-documentation-complete.log` for the corrected purpose comment.

### T233 final acceptance

Documentation builder 70116 exited zero at
`/tmp/ama-t233-documentation-complete.log`: 2497 maintained file pages, 9619
interfaces and all 491 referenced destinations. The successful 3314-input,
zero-debt inventory/header check from 72662 remains valid; its only later C
change adds the missing test-purpose comment and changes no interface or case
behavior. The renamed leaf's actual test passed at 74061. All jobs are collected.

The complete task is archived in DONE. The delivered CLI is
`ama-project init DESTINATION`; the separately exposed prepared-job/process API
is the independent orchestration foundation consumed by later build work.
T242 owns its accepted CMake/high-level test adapter, and acquisition/source
commands retain their assigned tasks. No unsupported command, new test ABI,
self-hosting requirement or renewed conformance matrix is claimed.

After this Markdown-only archival, regenerate Doxygen and verify accepted
non-Markdown fingerprints and staged bytes. Next ready task: T234.
