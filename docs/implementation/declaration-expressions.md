# Structural declaration Type expressions

## Tasks and authority

[T269](../../DONE.md#t269) owns the iterative syntax-tree foundation, preceding
[T226](../../TODO.md#t226)'s actual Type inference, selected producer evaluation
and runtime admission. Authority is
[TPL-15](../../spec/language/templates.md#tpl-15),
[TPL-02](../../spec/language/templates.md#tpl-02),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[TYPE-01](../../spec/language/declarations.md#type-01),
[CONST-03](../../spec/language/declarations.md#const-03) and
[COMP-01/02](../../spec/implementation/compiler.md#comp-01).
No specification amendment or new language form is proposed.

## Source boundary and plan

An ordinary declaration keeps its name and Type-head pair. Named inputs attach
one indentation level below that head. Each right-hand expression owns its own
nested inputs and optional named export selector. Initializers belong to the
outer declaration after the entire Type expression. Parsing records source
structure without finding modules or consulting category/signature metadata.

T269 will give the shared explicit-stack parser three distinct modes: static
value (inputs only), producer statement (inputs then outputs), and declaration
Type (inputs then at most one childless export selector). Nested graph inputs
remain static values; nested declaration operands retain declaration-Type
structure. Constant-read namespaces retain a simple head and optional named
selector; they do not acquire declaration input bindings. No ordinary static
output-binding aliases or standalone graph statements are introduced.

Steps: inventory all parser/type consumers; isolate reusable selector recognition
from phase/frame orchestration; connect declarations and restricted namespaces
to their respective modes; align obsolete syntax tests; check exact tree spans,
parent/child ownership, phase boundaries and initializers; test deep inputs and
all measured allocation refusals; run actual source/dependency consumers and
canonical document examples; register each maintained granule and validate
Doxygen before the independent foundation commit.

## Invariants and ownership

Source bytes and AST expression/binding ownership remain immutable and local to
the parsed module. Each explicit frame names one expression and its physical
head depth; each child belongs to one original input binding. Closing a frame
propagates its complete span to that binding and its parent, without consuming
a sibling or initializer. A selector terminates inputs at that expression level
and cannot own a child. Graph outputs never become declaration selectors.

Every scan transition consumes a line, pushes a frame for a consumed binding,
or pops an existing frame. Finite source therefore terminates without C-recursive
parsing or a depth heuristic. All partial frames/AST nodes use the existing AST
arena and ordinary failed-parse destructor. Recognition and source traversal
costs, actual failure partitions and consumer evidence will be recorded with
the implementation.

## Initial investigation

After T221 commit `40d17bd`, `parser_runtime_type` accepts only simple heads
and selected exports; both declarations and constant-read namespaces call it.
The existing graph expression parser already uses explicit frames and nested
input ownership, but permits no selected-value expressions. Dependency/event
collectors already traverse structural input children. Semantic ordinary-export
resolution currently creates a zero-argument independent producer root and does
not consume declaration inputs; this remains an explicit T226 implementation
owner rather than a syntax acceptance claim. The current canonical parser
composition failure at templates.md fence 3 is the first observed consumer.

## T269 implementation and results

The shared iterative driver now has an explicit three-value source-mode enum.
Declaration frames accept named inputs and then at most one foreign selector;
their nested input frames retain declaration mode. Producer frames retain their
existing input/output phases and their nested children remain input-only graph
values. The reusable allocation-free selector operation performs no lookup.
The former runtime-Type helper was renamed to namespace-type and remains the
restricted constant-read consumer; its obsolete public name/source was removed.
Declarations call the shared structural driver directly, without a wrapper or
a second expression implementation. AST ownership remains unchanged.

The first parser run correctly exposed the obsolete test forbidding a declaration
input block (`/tmp/ama-t269-parser-initial.log`). That case is now positive tree
coverage. The new assertion source initially used incorrect fixture struct
member names; corrected to existing `source->span` and `whole_list` without
changing production behavior (`/tmp/ama-t269-parser-declarations.log`,
`/tmp/ama-t269-parser-declarations-corrected.log`).

The complete parser parent passed (94190, `/tmp/ama-t269-parser-parent.log`),
including the canonical templates.md fence 3 formerly pending in T226. The
complete project closure leaf passed (37963,
`/tmp/ama-t269-project-discovery.log`). It verifies five authored nested heads
in original source order, exact selector/ordinary dependency roles, original
spans and shared target identity for the repeated leaf. This is source loading,
not semantic Type admission. New structural cases cover all ordinary sections,
selected nested inputs, aliases, parent/sibling/initializer spans, source-only
unknown callees, crossed phases, graph/namespace separation and failed partial
parses. The deep test now checks 1024 input levels in both graph and declaration
contexts; allocation sweeps include a complete nested selected declaration.

For B source bytes and E expression/binding nodes, mode/phase decisions add
O(B + E) work to lexical scanning and traversal: selectors consume a line,
input pushes follow consumed lines and every frame pops once. Dedented lines
are not rescanned as text while ancestor frames close. Owned AST/frame storage
is O(E), with active linked-stack depth O(D); arena ownership retains closed
frames until module destruction. Original source bytes are borrowed, not copied
into labels or selector records. The enum is a required internal source mode,
not a category inferred from a module name or runtime value.

Current jobs: parser-static ASan/UBSan session 71031, log
`/tmp/ama-t269-parser-sanitized.log`; actual generated integer/marker regression
session 45784, log `/tmp/ama-t269-generated-regression.log`. Per-file evidence,
final documentation and archival are still required. T269 remains active;
T226 remains the explicit owner of full semantic declaration integration.

## Safe semantic-consumer domain during the foundation task

A real frontend regression demonstrated an unchecked handoff: a declaration
could supply `.unknown < ./value` to a zero-input static producer and still
compile, because the old ordinary-export consumer created its independent
zero-argument root and discarded the new bindings. The failing assertion is
retained at `/tmp/ama-t269-unconsumed-input-before.log` (80504, exit 2).
This is an implementation integration gap, not an ambiguous language rule.

The existing consumer now rejects a supplied input list before that legacy
path, with the first original binding as its diagnostic. T226 explicitly owns
replacing this fail-closed domain boundary with complete argument resolution;
it must not preserve an unsupported-form branch after the general consumer is
implemented. The regression checks the specific refusal and original span.
This boundary changes no specification and does not claim valid producer
applications are semantically implemented by T269. Nested selected producer
values already fail in the existing source-role consumer rather than being
silently discarded. Ordinary constructor inputs continue through the existing
structural operand kernel; no fallback interpretation is added.

The guarded frontend composition passed (41072,
`/tmp/ama-t269-unconsumed-input-refusal.log`): the ignored-input regression now
refuses at its exact original binding, while all existing concrete/static-export
and generic frontend cases still pass. Parser ASan/UBSan passed (71031,
`/tmp/ama-t269-parser-sanitized.log`) with `detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`, frame pointers, `-fsanitize=address,undefined`,
`-fno-pie` and `LINKER_FLAGS=-no-pie`. No LSan pass is claimed; the counted
allocation sweeps are independent ownership evidence. A later change only
clarified the declaration-output error text and comments; the complete focused
parser leaf passed again (88612,
`/tmp/ama-t269-parser-final-diagnostic.log`).
The first verified generated regression passed (45784,
`/tmp/ama-t269-generated-regression.log`); a final build/execution after the
safe semantic-consumer boundary uses `/tmp/ama-t269-generated-final.log`.
All new/changed source, header, test-support and consumer inputs are registered
with actual named cases. The old runtime-Type file/API has no maintained code
consumer or ready entry; its historical conformance fingerprint is intentionally
unchanged pending T237. Final documentation acceptance is recorded at
`/tmp/ama-t269-documentation.log`; T269 remains unchecked until that gate passes.

## T269 final acceptance and next owner

T269 is accepted; earlier active checkpoints above are historical. Final
verified generated C passed (85367, `/tmp/ama-t269-generated-final.log`).
The full documentation gate passed (8848, `/tmp/ama-t269-documentation.log`):
3251 ready inputs, zero debts, 2450 maintained file pages, 9436 documented
interfaces and all 490 referenced destinations validated. Exact accepted
rendered-source and complete task-input/deletion snapshots are retained at
`/tmp/ama-t269-accepted-manifest.json` and
`/tmp/ama-t269-accepted-task-inputs.json`. No behavior job remains active.

The full task record is [DONE T269](../../DONE.md#t269). T226 is now ready and
owns all remaining semantic/executable declaration-expression integration,
including removal/replacement of the explicit unsupported producer-input gate.
Only narrative archival/status links change before the independent task commit.
Supported Doxygen regeneration at `/tmp/ama-t269-documentation-archived.log`
must agree with the final site/report and staged inputs, with accepted
non-Markdown files and deletions unchanged.

Final staged review corrected two inherited purpose-comment descriptions:
producer input independence is not a language-wide ban on declaration bindings,
and constant-read namespace restrictions do not restrict ordinary declarations.
An exact comparison excluding each first Doxygen block proves executable C
source bytes unchanged; body digests are retained in
`/tmp/ama-t269-purpose-comment-proof.json`. The final complete documentation
recheck is `/tmp/ama-t269-documentation-purpose-final.log`; no behavioral
implementation, test, registry or specification change accompanies this cleanup.
