# Actual implementation alignment with static modules

## Task, inputs and acceptance

[T203](../../../DONE.md#t203) starts at `99ffbed`, after T202's static-module
contract and T205's lifecycle clarification. The 2026-11-19 continuation request
also requires removing unused/obsolete implementation and its sole-purpose tests,
and thorough testing of current specification behavior. This audit changes
records and implementation instructions; source replacements belong to their
explicit dependency-ordered implementation owners. It grants no spec edits.

The authority is [TPL-02](../../../spec/language/templates.md#tpl-02),
[TPL-12](../../../spec/language/static-results.md#tpl-12),
[ARCH-01](../../../spec/implementation/compiler.md#arch-01) and
[ARCH-06](../../../spec/implementation/compiler.md#arch-06). Historical T168
findings are preserved in [the original audit](template-sprint.md); they are not
current evidence of syntax, metadata or execution support.

Acceptance requires actual source/test/consumer mapping for every affected
responsibility, explicit reuse/adapt/remove ownership, dated evidence, retained
fingerprints of unfinished T182 files, and a first implementation task with
independently observable checks. An unchanged reusable component need not be
rewritten. A test that merely validates abandoned syntax must be removed or
converted into a rejection case; it is not compatibility support.

## Planned steps

| Step | Purpose | Expected evidence | State |
|---|---|---|---|
| 1 | Capture the exact working baseline and owning contracts | Source/test inventory, dirty-input hashes, actual pipeline trace | Complete |
| 2 | Compare each current component with canonical rules | File/consumer/keep-adapt-remove map and replacement owners | Complete |
| 3 | Characterize real entry paths and relevant retained foundations | Focused actual tests, current-source probes, warning/lifecycle review | Complete; gaps and one incorrect test command recorded below |
| 4 | Accept and hand off | Complete mapping, honest limits, records/navigation checks and task commit | Mapping, fingerprint and initial navigation checks pass; final documentation and staged-byte checks are commit gates |

## Findings and checkpoint

The actual frontend owns a static inventory for early native admission, but
`compiler_frontend_finish` still invokes ordinary semantic analysis and concrete
planning. The inspected parser activates only the earlier five annotations;
it has no static-module kind. Its static AST still retains positional/omitted
bindings and explicit constructor signatures. These are concrete source gaps,
not a reason to alter the accepted specification. T206 owns the source/AST
replacement; later inventory/metadata owners must remove the corresponding
superseded semantics and sole-purpose tests.

The [input manifest](static-alignment-inputs.json) records SHA-256 fingerprints
of all 73 pre-existing unfinished T182 files, including its untracked components,
test support, local note and documentation registry. It is preservation evidence,
not acceptance. No source/test deletion or implementation acceptance has occurred
in T203. The actual source location for early admission is
`src/compiler/admission.c`; dependency expression collection is
`src/project/dependencies/discover/expressions.c`.

## Planned characterization checks

Run the root build and independent leaf-output trees without overlapping any
leaf with its parent. Use the existing default strict C11 leaf flags, O0/debug
and assertions; root defaults are O2. Capture each command's real completion
status separately. These checks characterize existing behavior, not support for
T202's new forms. Inspect source/consumer handoffs independently of passing cases.

| Check | Command from repository root | Expected observation before running |
|---|---|---|
| Root build | `make -j4` | Current working inputs build; do not treat an incremental build as resolution of the known warning |
| Warning reproduction | Compile `connections/typed/scope/prepare.c` independently with root O2/warning flags | Establish the actual diagnostic and its source; no source fix in an audit |
| Earlier static syntax | `make -C tests/unit/parser/static -j4` | Existing old-interface tests characterize code to remove/replace, not accepted grammar |
| Retained term congruence | `make -C tests/unit/semantic/static/constraints/equivalent -j4` | Exact finite-model congruence and refusal checks remain useful |
| Unfinished typed association | `make -C tests/unit/semantic/static/connections/typed -j4` | Actual source-owned terms/owner joins characterize reusable T182 foundations |
| Creation cache | `make -C tests/unit/semantic/static/guards/cache -j4` | Incoming ancestry/cache checks remain required, with source setup to migrate |
| Definition graph oracle | `make -C tests/unit/semantic/static/guards/definition-model -j4` | Exact definition/equality-cycle distinction can be retained |
| Ordinary frontend | `make -C tests/unit/compiler/check -j4` | Real concrete frontend remains functional; not a template execution test |
| Owned child and host | `make -C tests/unit/library/nested -j4`; initially also attempted a direct consumer invocation | Real compiled C child/native wrapper cleanup, failure and ownership checks; actual output ownership is clarified below |
| Current source probes | Invoke the newly built compiler on isolated current-spec examples | Bare ordinary Type placeholders/static graphs/selectors expose real unsupported boundaries without changed expectations |

## Actual component and removal map

The [exact path map](static-alignment-map.json) inventories 1730 source, test,
fixture, support and contract paths across 25 responsibility boundaries. Each
path has one primary owner/disposition, including all 73 dirty inputs. Shared
interfaces remain explicit downstream dependencies, not permission to rewrite
every file in a directory. This is an ownership inventory, not a claim that every
function in the downstream runtime/tool consumers was inspected or tested here.
The following findings come from actual implementations and their caller/test
interfaces, rather than filenames or earlier task descriptions.

| Boundary and actual evidence | Retain / adapt / remove | Current owner and required consumer |
|---|---|---|
| `lexer/tokens/recognize.c`, `parser/module/{sections,recognition/*}.c`, `ast/static.h` | Preserve exact names/layout/arena ownership. Add the distinct `@...` token and module kind; remove obsolete positional/omitted static syntax and explicit formal-signature syntax, not ordinary runtime positions. | T206; lexer, section parser and immutable AST compositions |
| `parser/declarations/section.c`, `parser/static/{formal,statement,expression,binding}.c` | Remove mixed ordinary static formals/results/projections and their acceptance cases. Implement graph pairs/producer selections and the single named ordinary export selector; preserve finite iterative child parsing and source spans. | T206; parser declarations/body/constant source consumers |
| `inventory/explicit.c`, `classify/candidate.c`, `scope/lookup.c`, `events/ast.c` | Old inventory collects explicit formals and treats aliases as visible only after their first receipt. Replace with ordinary Type introductions and whole-static-graph public binder collection before free inputs. Retain exact definition identities and source-only statement cataloguing. | T207; source inventory, native admission and later term environments |
| `signature/compare.c`, `constraints/metadata/{signature,refine}.c`, `applications/check/expected.c` | Preserve complete actual metadata and exact equality where comparing actual Templates. Remove the assumption that an input's expected signature exhausts its actual residuals; infer minimum requirements from all uses. | T208/T209; term equality and partial application, not first-actual selection |
| `terms/nominal/construct.c`, `constraints/solve/*`, `constraints/equivalence/query.c` | Retain immutable original origins/capture partitions, finite equations, occurs checking and sound owner congruence. Rebind to the new inventory/metadata without treating a producer as an ordinary nominal application Type. | T208/T209; T182 field joins and T183 candidate keys |
| `bindings/request/static.c`, `applications/recover.c` | Remove positional/skipped static normalization. Recovery currently depends on a declared exhaustive residual set; replace that contract with current uniquely justified nominal recovery while retaining all actual residuals/captures. Ordinary runtime normalization remains valid. | T209; named static applications, T211 producer equations |
| `guards/path/enter.c`, `guards/cache/check.c`, `guards/definition/check.c` | Retain exact finite graph searches, pending comparison keys, original provenance and definition/equality separation. Adapt old expression/output-phase classification to ordinary constructors versus separate static producers. | T210; every T183/T211 cache/expansion path |
| Source identity encoder/registry, `codegen/model/identity.c` | Retain length-framed full identity encoding and collision checks. Generic name registration and guard-local nominal interning are not a closed-specialization registry; that service and pending validation state are still absent. | T183; T184 closure and T188 generated identities |
| `results/{source,evaluate,use,cache}/*`, `publication/closure/graph.c` | Retain checked dependency witnesses and closure worklists. Replace ordinary-module static results/output ordinals with whole named graph results and complete producer inputs; do not publish a candidate as executable. | T211; T212 independent export selection |
| `results/pattern/apply.c`, `results/plan.h`, `results/model.h` | `static_result_pattern_apply` has no production caller: all invocations are in its own pattern leaf. Remove this unused preview API and its cached application field/sole-purpose tests. Keep `pattern/prepare.c`: `results/expression/prepare.c` actually uses its source/environment operand preparation. | New T214 before T206; verify live preparation through actual expression consumers |
| `constants/declaration/bind.c`, `constants/source/select.c` | Retain original constant membership/storage checks and declared-Type evidence. Replace old inline static namespace applications with the independently concrete selected export; preserve direct qualified-member splitting and runtime `>>` positions. | T212; T182 typed connections and T186 startup |
| `connections/runtime/*`, `connections/typed/scope/{prepare,merge}.c` | Retain source-order receiver identities, scoped field terms and exact congruence joins. Remove computed-interface preview assumptions; keep ordinary metadata separate from guarded body expansion. Fix the confirmed O2 return-contract warning in the responsible task. | T182 after its new prerequisites; actual call/default/reference/list connections |
| `compiler/admission.c`, `semantic/static/native/admission.c`, `compiler/authoring/check.c` | Retain early own-formal native rejection and frontend reverse ownership. Adapt the inventory source model before headers; no generic native ABI or runtime static slots. | T207/T190; compiler native-admission and scaffold consumers |
| `compiler/frontend.c`, `semantic/{analyze,names/build,resolve/types}.c` | The actual executable path deliberately rejects unresolved static syntax and maps one source module to one concrete SemanticType. It does not run term/application/guard/result closure. Replace that boundary through the planned orchestration/admission; do not delete its protection before a checked replacement exists. | T184/T185; compiler/check then lowering |
| `project/dependencies/discover/{ast,expressions}.c` | Retain iterative actual expression-path traversal and sorted exact requester spans. Add graph pairs/producer heads/selectors and kind validation; descriptive static names remain nonpaths. | T213; real configured-root closure, followed by T191 invalidation |
| Planning, emission and constant startup | Preserve typed physical slots, labels, native entry and reverse lifecycle. Their semantic identity inputs and source-only code/constant keys require specialization-aware adaptation. | T186–T188; actual generated compilation/execution, not substring checks |
| Native, diagnostics, source delivery, editor, relocation and C library | Retain their common frontend and checked transaction/transport boundaries. Adapt resolved static identities/actual path spans, without an alternative parser, generic native ABI or static export roots. | T189–T194; exact files and existing entry commands are in the path map |

T195 still needs real generic algorithm projects over the unchanged intrinsic
lists; none is claimed implemented by this audit. T196 owns the current
rule/acceptance/per-file evidence reconciliation; T197/T198 remain release and
final generated-reference gates. Shared test source lists remain live harness
infrastructure: their consumers update them with their actual source changes.

The old parser leaf explicitly runs
`it_retains_positional_input_and_output_skips_as_distinct_bindings`,
`it_composes_static_interfaces_with_runtime_declarations_in_both_implementations`
and `it_parses_deep_signatures_without_c_recursion`. Their abandoned forms must
become rejections or disappear with their implementation. Useful properties
such as finite-depth iterative traversal and refusal cleanup must instead be
exercised on current graph syntax. Scope tests requiring forward-only alias
visibility need whole-graph replacements; recovery/metadata tests enforcing
exhaustive expected signatures need minimum-requirement and extra-residual cases.
Actual complete Template equality remains exact and must not be weakened.

### Preserving T182 without retaining its dead preview

The captured dirty inputs belong to unfinished work, not to this audit's code
commit. The user subsequently expressly requires removal of unused/obsolete
code. T214 therefore has authority to remove the specifically identified dormant
API and its tests while preserving the original fingerprints and historical run
results. The retained source-only operand preparation has an actual production
consumer and is not removed merely because the adjacent preview API is unused.
The remaining lexical/typed/congruence foundations retain explicit T182/T209
owners; their source setup and current-spec assumptions require revalidation.

### Lifecycle clarification and warning review

The maintained native adapter's `close_child` disposes its exclusively owned
child through the public C API and never resumes it. `open_child` releases a
partially acquired child before reporting failed native construction. The actual
host checks yielded-child abandonment, two independent owned children, immutable
reply lifetime and measured parent-allocation failures. This agrees with T205's
owned-child rule. It does not independently establish that an unrelated peer C
survives every outer-failure cleanup: T194 must add the explicit A/W/B plus peer C
composition and retain existing callback/code-lifetime checks.

The root build exits 0, but that incremental result does not repair T182's
warning. The independent command was:

```sh
gcc -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -Isrc \
    -c src/semantic/static/connections/typed/scope/prepare.c \
    -o /tmp/ama-t203-scope.o
```

It exits 0 and reports `value may be used uninitialized` at publication of the
typed scope. Inspection confirms that `owner` records a refusal by returning an
externally defined always-false `static_connection_fail`; the optimizer cannot
prove that every true return initializes `value`. T182 must record the same
failure and return literal false on refusal, preserving the real contract rather
than inventing a default owner. No warning suppression or source fix is hidden
in T203. Exact log: `/tmp/ama-t203-warning.log`.

## Collected verification and actual limits

All seven valid planned leaf invocations exit 0 with the captured working inputs:
parser/static, constraints/equivalent, connections/typed, guards/cache,
guards/definition-model, compiler/check and library/nested. Commands and separate
logs are `/tmp/ama-t203-check-0.log` through `-6.log`; root build is
`/tmp/ama-t203-build.log`, exit 0. The congruence leaf checks all five partitions
of three variables and 1280 tagged-term comparisons, a 1024-level shared DAG,
unknown-future-equal controls and every measured scratch refusal. Cache and
definition-model tests retain independent graph outcomes. These are bounded
retained-component characterizations, not current template execution acceptance.

The extra `make -C tests/unit/library/nested/consumer -j4` exits 2: the consumer
directory has no Makefile. This was an incorrect audit command, not a passed
test. Inspection confirms that the actual nested producer builds and runs this
public-header-only C host, whose `run` explicitly executes all three ownership/
failure cases; the parent command already exited 0. Use that real owner command,
not an invented leaf or another test harness. The expected injected runtime
failure printed inside the successful parent is distinct from this invalid
separate invocation. Original failed log: `/tmp/ama-t203-check-7.log`.

The [source probes](static-alignment-probes.json) retain every exact source,
target expectation, command, observed exit and diagnostic. The concrete ordinary
control is accepted. A valid ordinary template call is rejected at the known
concrete semantic gate; two valid static-export examples are rejected before the
selector can be represented. The obsolete standalone ordinary static formal is
rejected only at semantic checking, although the new parser must reject its
syntax. These are explicit T184/T206/T212 gaps, not changed expected results or
an implementation ambiguity. No full conformance, sanitizer rerun, whole test
suite or new-spec generated execution is claimed.

## Final review and handoff

The actual ownership/replacement map and characterization are complete. Initial
record checks pass: 31 documents, 1218 local links, all 214 canonical rules and
11 EBNF blocks/108 productions; the task graph has no cycle and all 37 template
acceptance areas retain current owners. Every one of the 73 pre-existing hashes
still matches; all 1730 mapped paths exist and have unique ownership. The user
directive is recorded in AGENTS; the contributor formatting guide now points to
the existing exclusive static-module rule instead of listing only old markers.
No specification change is made.

No compiler/test jobs remain active; session 12646 was collected with wrapper
exit 1 because of the invalid extra consumer command, and the seven successful
child commands have their separate exit-0 records. The two errors must not be
conflated: unsupported valid new language forms are recorded implementation gaps,
while the invalid direct consumer command was an audit invocation mistake.

T203 is archived with its scope and limitations. Its final navigation/task checks
pass on 32 documents, 1239 links and 26 remaining tasks with 79 prerequisite
edges. The initial archival check caught one stale T203 link in AMB-013; it now
points to DONE. The private graph negative control initially named the archived
task and was moved to the live T214/T206 cycle; cycle rejection remains checked.
The successful reruns include all grammar/group and graph/ownership negative
controls. Doxygen generation runs on a separate `99ffbed` archive plus only the audit
records/instructions/queue changes, excluding unfinished implementation and its
coverage registry. Successful generation and exact staged-snapshot agreement
are required before committing; actual counts and tool versions accompany that
commit. No implementation fingerprints are refreshed. Then continue with T214's
authorized removal and real consumer tests, followed by T206. No unresolved
specification question or spec edit has been introduced.
