# T001 findings and follow-up decisions

[Audit evidence](README.md) distinguishes passing historical behavior from
current acceptance. These findings route implementation work; they do not amend
the specification. Canonical links below are the contracts to implement.

## F01. The source identity and spelling model is outdated

[SRC-01/SRC-03](../../../spec/language/source-and-names.md#src-01),
[SYN-06](../../../spec/language/source-and-names.md#syn-06) and
[NAME-01](../../../spec/implementation/compiler.md#name-01) require root-kind
identity and multiword names with exact punctuation spacing.
`src/source/module/resolve/logical.c` accepts one initial dot followed by
single-word identifiers. `src/source/identity/encoding/encode.c` consequently
does not encode the new root-family distinction. `src/parser/access/names.c`
recognizes one identifier word; `src/parser/body/statement.c` strips only the
execution marker, leaving the now-required separator to fail access parsing.

The probes confirm that `worker !` and a multiword declaration fail while the
obsolete `worker!` succeeds. Preserve strict UTF-8 decoding, span storage,
collision checks and physical submodule handling; replace their outdated
recognition/identity consumers through T014–T028, not a compatibility grammar.

## F02. The list dispatcher and completion state are retired

[LIST-04](../../../spec/language/lists.md#list-04),
[LIST-03](../../../spec/language/lists.md#list-03) and
[LIST-13](../../../spec/language/lists.md#list-13) require typed intrinsic
methods, membership records and live traversal state.
`src/semantic/calls/lists.c` instead binds four input and three output positions.
`src/runtime/lists/list.h` exposes `AmaListAction`/`AmaListRequest`;
`src/runtime/objects/objects.h` stores retained `refused`/`outcome` list state;
list copying copies those obsolete fields. CLI options and semantic library
configuration still select action/index/condition/outcome types. The standard
library publishes the retired action/outcome modules.

`src/codegen/control/emit.c` explicitly emits a technical failure for old
`CONTROL_LIST_ACTION` operations. Passing standalone mutation tests therefore
never established complete executable list support. T136 removes the obsolete
contract and its registrations/expectations throughout the consumers. Preserve
contract-independent growth, checked bounds, lifecycle and ownership algorithms;
T075–T084 supply the new membership/service/traversal implementation.

## F03. New declaration, result and control models are absent

[TYPE-06](../../../spec/language/declarations.md#type-06),
[NAME-06](../../../spec/language/declarations.md#name-06),
[CALL-09](../../../spec/language/calls-and-results.md#call-09),
[REF-01](../../../spec/language/values-and-references.md#ref-01) and
[FLOW-05](../../../spec/language/lists.md#flow-05) have no corresponding complete
models in the current AST/binder/control storage. The namespace builder collects
only `AST_DECLARATION`; receivers are looked up as already existing fields.
Sources always require an access operand after `&`, excluding self-reference.
The AST has flat call bindings and no foreach/result-action tree node.

The body parser accepts only `?`/`{` after a bare condition, excluding negative-only
branches. `src/lowering/control/statements.c` routes an empty loop's true edge
back to its condition; [FLOW-02](../../../spec/language/control-flow.md#flow-02)
instead requires one check and exit. No endless-loop probe was run. The existing
control graph itself establishes this gap. Receiver, self-reference, foreach and
negative-only source probes independently confirm missing current acceptance.
T023–T044/T063–T084 must replace these models and add exact boundary tests.

## F04. Layout validation skips newly forbidden bytes and indentation

[SYN-01/SYN-02](../../../spec/language/source-and-names.md#syn-01) ban TAB
everywhere and define comment/block boundaries before draft exclusion.
`src/lexer/layout/validate.c` exempts literal/comment payload TABs;
`src/parser/module/sections.c` skips validation entirely for inactive files.
`src/lexer/layout/line.c` trims trailing code spaces and treats space-only lines
as blank. `parser_significant` skips standalone comments without preserving
their block-closing effect. Existing integration tests explicitly accept a TAB
inside literal payload.

The probes confirm accepted comment/draft TABs, trailing code spaces and a
space-only line. T022–T028 must replace the old expectations, preserve actual
literal/source bytes and validate draft lexical rules without fully parsing
unfinished bodies. These are implementation gaps, not unclear language choices.

## F05. Generated runtime ownership is not context-isolated

[PAR-01/PAR-07](../../../spec/language/async-and-parallelism.md#par-01),
[RT-01](../../../spec/implementation/runtime.md#rt-01) and
[REF-02](../../../spec/language/values-and-references.md#ref-02) require distinct
mutable context state and checked host identities.
`src/codegen/project/host/context.c` emits a mutable static
`ama_project_owner`, rejecting a second host while the first remains owned.
Generated pool/constant descriptors also need context-owned materialization.
The old continuation host test expects that second-host refusal, so its success
does not demonstrate independent contexts. A plain `host->executing` Boolean
does not establish a concurrent admission guard.

Local `AmaHandle` triples are useful inside one context; they are not sufficient
host tokens for selecting among contexts. T006/T013/T052/T061 fix the ownership
boundary before new consumers; T100–T106 add actual parallel acceptance. Do not
hide ordinary Ama references in native payloads as a workaround.

## F06. Several required facilities have no implementation yet

[EMBED-01–EMBED-08](../../../spec/implementation/c-library.md#embed-01),
[ASYNC-04](../../../spec/language/async-and-parallelism.md#async-04),
[PAR-03](../../../spec/language/async-and-parallelism.md#par-03) and
[TOOL-01](../../../spec/implementation/tools-and-conformance.md#tool-01) describe
facilities absent from the current CLI, AST/runtime interfaces and supplied
library. The CLI provides check/status/native/emit; source-included custom-host
continuation tests do not produce independently linked Ama libraries. There is
no export/transport model, foreign-context native adapter, enrollment/reactor/
ready-queue/cancellation implementation, parallel message channel, or relocation
transaction/recovery/editor/debugger interface.

Keep their tasks open: T090–T115/T123–T131 are implementation work, not presumed
support from synchronous `%` or an ordinary C helper. No new syntax is needed
to invent those facilities during the audit.

## F07. Library acceptance needs the new integer and failure contracts

[LIST-04](../../../spec/language/lists.md#list-04),
[PROTO-03](../../../spec/implementation/native-interface.md#proto-03) and
[ERROR-01](../../../spec/implementation/diagnostics.md#error-01) require the
nominal `..integer` size bridge and one condition-capable application failure
value with its declared description result.
The supplied library uses `.std.index` and old list selector types; it has no
required `..integer`/`from_size` implementation. The technical/refusal snapshot
modules are not acceptance evidence for the new application failure value.

Retain useful checked scalar, finite-value, status and refusal-reader behavior,
then adapt it through T085–T089. Test the full supported size range, one selected
constructor and false/true/false failure transitions with description lifetime.
Do not turn technical decoder/allocation failures into ordinary failure values.

T085 resolves the scalar/size-bridge portion through the
[canonical integer and standard-root scalar library](../scalar-library.md).
T086 remains responsible for the condition-capable owned failure value.

## F08. Retained code lacks file documentation and complete boundary evidence

[ARCH-01–ARCH-06](../../../spec/implementation/compiler.md#arch-01) and
[TEST-06–TEST-08](../../../spec/implementation/tools-and-conformance.md#test-06)
apply to the retained implementation, not only future files. All 428 maintained
C/header files lack `@file`; successful Doxygen extraction used `EXTRACT_ALL`
and is not proof of comments. Actual preprocessor consumers cover every
production `.c` except the CLI entry, which is exercised as a built executable.
Other missing direct consumers must be checked through their real native/output
workflows or receive explicit tests. Include presence is not behavioral coverage.

Source review identifies distinct phases still combined in driver option
parsing, native artifact publication, control/descriptor emission, object
construction/clone preparation, input preparation and value/list transfer.
T138–T144 separate those actual responsibilities, with invariants and before/
after component/composition evidence, rather than splitting by a line threshold.
T145–T151 document the remaining components and their test mappings; T005
establishes strict source/rendered-link and coverage checks first.

## F09. Derived descriptions still assert superseded contracts

`docs/implementation/components.md` describes an earlier replacement-only reset
and says no production reuse is approved; it is not the current T001 outcome.
`stdlib/README.md` links removed root documents and publishes the old list
dispatcher contract. The undecided list-conversion design note is superseded.
These derived descriptions cannot override `spec/` or justify retaining old APIs.

T002 refreshes the responsibility/contract map. T137 removes conflicting derived
descriptions and obsolete proposals while retaining useful execution evidence
with an explicit historical scope. No file in `spec/` is edited to match code.
