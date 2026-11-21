# Early native static-parameter admission

T174 is accepted on input 11d0b7e (2026-11-18). T172 supplies complete own-formal identities;
T173 supplies distinct interface sequence contracts. The authority is
[TPL-13](../../spec/language/templates.md#tpl-13) and
[CABI-06](../../spec/implementation/native-interface.md#cabi-06).
No specification change is authorized or needed for this task.

## Contract and planned steps

A native implementation cannot have any own Type/Template formal, including an
unused formal, an input pattern or a Type first introduced through results.
Binding every application concretely does not change its definition. Use the
completed scoped inventory; neither spelling `...` nor a returned constructor's
foreign residual signature is a native own-formal test. Received aliases and
static result binders contribute no own input positions.

1. Add an allocation-free admission component consuming a valid StaticInventory.
   Choose the first prohibited native definition in the deterministic loaded
   inventory order and its original formal ordinal zero. Report the original
   `@C` marker and the parameter introduction. Retain only source/project borrows
   in the diagnostic, never an arena-local temporary related record. Test every
   introduction category and allowed nonformal controls independently.
2. Integrate that source admission after exact closure loading and before input
   tracking/ABI authoring/header preparation, on normal, explicit-required and
   preloaded-catalog paths. Frontend owns the static inventory/diagnostic through
   rendering and destroys it before its source Project. Initialization credits
   precede fallible collection. No default specialization or C companion is made.
3. Compose real check-only/native-authoring/scaffold paths, including fully bound
   uses of an invalid native definition, original diagnostics, no generated
   artifacts on refusal, and preservation of existing concrete native behavior.
   Test every actual new frontend allocation refusal, relevant parent/regression
   suites, sanitizers, per-file/header coverage, local/source/generated links.
   Archive only after acceptance and commit this task separately.

Known stage limits: admission is only the own-formal policy, not field type
closure or generated template/native interoperability. A constructor returned
as static metadata may retain its own residuals without making its producer
native-generic. Runtime native field types still require unique concrete types
before header/layout publication; that later integration is owned by T184/T190.
No unresolved type is converted to void pointers or a guessed specialization.

Initial source inspection: all ordinary/required loads pass through
compiler_frontend_load_required; preloaded catalog checking has a parallel load
entry. Both currently capture inputs before semantic completion. Authoring uses
the ordinary load before preparing its proposed ABI. Admission belongs between
successful project parsing and capture on both paths; hiding it in input I/O or
waiting until scaffold publication would obscure the policy boundary.

Initial checkpoint (superseded): contract/planning, no active jobs. Next: implement and test
the pure own-formal admission and its original-source diagnostic. T174 is not
complete; existing T172/T173 evidence does not stand in for this task's checks.

Pure admission passed five cases under strict C11 in session 22639
(`/tmp/ama-t174-admission.log`): every own-formal introduction, fully bound uses,
nonformal/returned-residual/alias controls, original formal/module ordering and
diagnostic validity after inventory scratch destruction. Admission performs no
allocation or source/inventory mutation and borrows only persistent Project data.

Frontend integration now owns the static inventory and bounded native diagnostic,
sets initialization credit before collection, and destroys static metadata before
Project/source. Ordinary/required and preloaded-catalog loads call admission before
input capture; authoring therefore cannot prepare a proposed ABI/header first.
The explicit frontend source list is shared by its nine existing leaf consumers
instead of repeating a new translation-unit edit in each future consumer.

The first strict whole-frontend build exposed the existing non-exhaustive
lowering switch after T170 added three AST static kinds. Added explicit unresolved-
static refusal with original diagnostics and a direct parsed-node boundary test;
this supplies no T185 template lowering and emits no runtime node. It is the
required strict-build/safety prerequisite, not a warning suppression. The updated
frontend admission leaf passed five cases
(`/tmp/ama-t174-frontend-admission.log`): normal/authoring introductions before
invalid ABI/proposal capture, required/catalog routes, existing concrete native
checks, every actual new allocation refusal and all three lowering guards.

Next: actual CLI check/authoring refusal and absence of generated companions,
then compiler/library/relocation consumers and concrete check/scaffold regressions.
No whole native template field-generation claim is made; T190 owns that acceptance.

Regression checkpoint (superseded by final acceptance): no functional edits while these jobs run.
- Compiler parent, strict C11 warnings-as-errors: session 97654,
  /tmp/ama-t174-compiler-parent.log.
- Actual CLI check/native-create/overwrite refusal: session 46415,
  /tmp/ama-t174-cli.log; maintained fixtures are copied to isolated scratch.
- Library consumer parent: session 53665, /tmp/ama-t174-library.log.
- Source relocation consumer parent: session 58056, /tmp/ama-t174-relocation.log.
Collect every result before further functional source edits. Remaining acceptance
also includes existing concrete check/scaffold commands, sanitizer/failure gates,
named per-file/header registry evidence, local/source/generated documentation,
archival and the individual task commit. No native template emission is claimed.

Regression update: actual CLI refusal passed session 46415
(`/tmp/ama-t174-cli.log`), including concretely bound uses, --create/--overwrite,
required original diagnostic contexts and absence of companions/generated output.
Compiler parent session 97654 stopped in the old T170 unresolved-static guard
assertion: two cases apply static inputs to a known nongeneric Type and now get
TPL-07's earlier precise error from lexical admission, rather than the later
legacy "Static syntax requires resolution" message. The other three cases still
belong to the legacy unresolved-static boundary. Update those exact expected
messages/stage credits, preserving rejection; do not weaken checks or change
semantics. Library 53665 and relocation 58056 are still running; collect before
functional edits. The failed parent remains /tmp/ama-t174-compiler-parent.log.

Library consumer parent passed session 53665 (/tmp/ama-t174-library.log).
Relocation 58056 continues its existing allocation/fault matrix. Its production,
fixture and Make inputs remain unchanged. The failed compiler/check expectation
is an independent leaf not read by that job; only that test's exact expected
message/stage credit was updated (three legacy unresolved forms, two earlier
known-Type application errors). No compiler behavior or active relocation input
was changed to obtain acceptance. Re-run the compiler parent on that correction.

Existing concrete CLI checks and native scaffold/create/refresh/overwrite
regressions also passed (/tmp/ama-t174-concrete-check.log and
/tmp/ama-t174-concrete-scaffold.log). New pure/frontend/CLI files now have twelve
named cases in the per-file registry, with independent header probes. Prior
compiler/library/relocation consumer case ownership is preserved; registration
alone does not certify an unfinished run.

The corrected strict compiler parent passed session 61310
(/tmp/ama-t174-compiler-parent-final.log). Pure/native frontend sanitizer runs
also passed session 51379 (/tmp/ama-t174-policy-sanitizers.log and
/tmp/ama-t174-frontend-sanitizers.log), with ASan/UBSan and strict C11; counted
teardown covers allocation prefixes and LeakSanitizer remains disabled for the
established container limit. Existing concrete CLI and library results remain
valid because only the independent compiler/check test expectation changed.

Documentation completeness passed session 42101: 2456 ready inputs, zero debt
(/tmp/ama-t174-doc-complete.log). Local audit passed 208 Markdown files, 2890
links, 214 canonical rules and 108 unchanged grammar productions
(/tmp/ama-t174-links.log). Relocation session 58056 still runs its finite
application-allocation matrix: the maintained loop is bounded by the measured
1340 allocation positions, with actual apply/rollback/committed cleanup per
position. No errors are reported so far; do not claim it passed until collected.

Production/test inputs are now immutable during generated documentation. The
Doxygen run uses /tmp/ama-t174-doxygen.log. Collect it and relocation before
source/comment changes, final acceptance or archival. T174 remains unfinished.

Generated Doxygen passed session 60489 (/tmp/ama-t174-doxygen.log): 1850 maintained
file pages, 7077 documented interfaces, all 420 referenced canonical destinations
and 5825 total pages, using Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0.
Actual HTML destinations and maintained-input stability were checked. Supported
source navigation `code --goto /home/u/repos/ama/spec/language/templates.md:683`
also returned success at the exact TPL-13 definition; no GUI click is inferred.

Relocation completed its application allocation matrix and interruption suite,
and is now in the final recovery/failures leaf. Its overall session 58056 is
still outstanding; no full-parent success is claimed yet. Maintained production
and test bytes stay unchanged while it runs. New/untracked maintained code and
fixtures also passed an explicit whitespace review before staged acceptance.

Final functional acceptance: the complete relocation parent passed session 58056
(/tmp/ama-t174-relocation.log), including all application allocation refusals,
interruption checkpoints, every pending/terminal commit/rollback filesystem
refusal and allocation refusal, independent recovery retries and resource counts.
All remaining jobs are now finished. No incomplete regression was skipped or
marked successful early. This completes the required consumer acceptance after
inserting the new source admission stage.

The pure policy costs O(M + S) time for M module records plus the first prohibited
native module's S symbols, with O(1) additional storage and no allocation. It
selects ordinal zero from the already verified dense formal order, independent
of explicit/implicit chain grouping or actual binding traversal. The frontend
collects once, records initialization before fallible work and retains both
inventory and diagnostic storage through rendering. Every partial owner is
released before borrowed Project/source. No semantic/header/native generation
runs after refusal; all fallback specialization/type-erasure paths are absent.

T174 is accepted on input 11d0b7e and archived with full original criteria and
this evidence. Final archive-input documentation refresh is logged in
/tmp/ama-t174-doxygen-archive.log; collect an outstanding refresh before the
individual task commit. T175 follows. Field uniqueness/concrete closure and
actual template/native generation remain their existing T184/T190 work; this
policy task neither changes the specification nor claims that later support.

## Current ordinary-call fixture alignment in T182

The whole-project regression run found that the CLI fixture still used a bare
static input and inline static bindings in an ordinary variable declaration.
Those source forms were removed by the static/ordinary split. The real `--check`
command therefore stopped in syntax parsing before the policy tested here.
The maintained fixture now introduces `...type` through an ordinary reference
input pair and supplies a concrete owned `.plain` value through `& known` in an
ordinary call. Both the entry-native and reachable-native cases must still
report the own-parameter policy with the original `@C` and introduction locations,
and authoring must still leave every companion/header publication path absent.
No native implementation policy or compiler diagnostic is weakened to accept the
obsolete source form. T190 retains the later complete native authoring consumers.
