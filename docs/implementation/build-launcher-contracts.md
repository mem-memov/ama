# Build and launcher handoffs

## T244 scope and evidence boundary

This internal implementation contract follows T243/T229/T230 and the accepted
project/configuration foundations through T234 (`dc284fb`). It refines the
[task plan](cmake-launcher-plan.md); the entire [specification](../../spec/README.md)
remains authoritative. The data models below are implementation handoffs, not
new Ama syntax, another `.abi` protocol or an assertion that future commands
already work. T245–T257 implement and test them. No dormant C interface is added
by this documentation task.

Read-only investigation traced actual callers at `dc284fb`. Existing tests below
are migration witnesses with real current consumers. They are not claimed to
prove the proposed replacement before its implementation. No compiler/runtime
code changes or new behavioral acceptance claims belong to T244.

## Current owners and migration ledger

| Responsibility and current consumer | Treatment and implementation owner | Current witness and replacement acceptance |
|---|---|---|
| `DriverOptions` in [options.h](../../src/driver/options/options.h), exact request recognition in [request.h](../../src/driver/request/request.h), emission through [generate.c](../../src/codegen/project/generate.c) | T245 validates explicit mode/component, borrows the checked concrete root, captures exact literal capabilities and emits owned delivery metadata. T240 separately owns runtime token admission. Keep exact checking/library/editor requests with their current consumers. | `tests/unit/driver/request` exact-command cases; `tests/unit/codegen/assembly` real emission and allocation refusal. Replacement adds both output modes and malformed request/schema refusals. |
| Selected companion/cache graph in [ProjectInputs](../../src/project/inputs/inputs.h) and source/module resolution | T239 extends the existing selected-input capture to `.cmake`, declared resources and optional effective README. Retain the common resolver; T235 consumes the same closure for delivery. | `tests/unit/project/inputs`, source module/configuration suites. Replacement checks presence vs empty, ignored dependency copies and native/static/specialized closure membership. |
| Native author-file preparation/publication under `src/native/scaffold/output/` | T246 adapts the existing author-preserving artifact transaction; T247 separately consumes the authored CMake function. Keep ownership/publication children with native authoring and generated-output consumers. | `tests/unit/native/scaffold/publication`: new-file, replacement, rollback and lock failures. Replacement covers `.cmake` omission/existing bytes and comments-only generation. |
| Root startup: [ama_context_execution_start](../../src/runtime/context/execution/begin.c), [ama_program_start](../../src/runtime/program/construction/start.c) | T248 adapts fresh-root startup to supplied recipes. Retain acquired existing-root entry/resume and their distinct interface-reset semantics. Runtime remains one lease and label dispatch, never a C call per Ama module. | `tests/unit/runtime/program-construction` real self construction and failed-prefix cleanup; `tests/unit/runtime/contexts` independent contexts/reentry/cleanup. Replacement proves supplied/default choice once in declaration order. |
| Exact host schema/recipe construction in [library/model.h](../../src/library/model.h), `src/library/resolve/`, [transport/recipes.c](../../src/library/transport/recipes.c) | Retain the stateful library/export consumers. T245/T256 reuse exact nominal/schema validation and checked fresh construction; a byte-file decoder is an additional explicit adapter, not a memory cast. | `tests/unit/library/transport`: actual typed native inputs/retained reads; `tests/unit/library/execution` exports/resume. Replacement adds exact byte-format/decoder admission and malformed data/failure cleanup. |
| Persistent generated host in `src/codegen/project/host/`, fixed [launcher.c](../../src/codegen/project/host/launcher.c) | T250 adds a fresh one-shot operation; T241 replaces the fixed `main(void)` artifact route with a matching header/implementation and forwarding `main(argc, argv)`. Preserve stateful exported lifecycle entry points with library consumers. Remove superseded fixed-launcher code/tests in T241's migration commit. | `tests/unit/codegen/assembly`, actual integration compile/run; library execution/nested consumers retain stateful behavior. Replacement tests repeated/concurrent isolated calls, both modes, yield and cleanup. |
| Internal checked hash identities and [library identities](../../src/library/resolve/identities.c) | Retain internal/native/library naming and collision checks. T251 adds the separate exact EMBED-09 public name encoder; it does not change hashed-name semantics globally. | identity/library suites. Replacement checks full Unicode/version/path/component encoding, bounded arithmetic and toolchain identifier limits. |
| Exact native/control symbol rejection in [verification/entries.c](../../src/codegen/toolchain/verification/entries.c), `src/native/toolchain/` | Retain exact identities, non-LTO evidence and parser checks. T254 adapts entry evidence to executable vs inclusion mode and the actual CMake-built artifacts. Replace the unconditional `main` assumption in [verify.c](../../src/codegen/toolchain/verify.c) when those consumers migrate. | `tests/unit/native/toolchain/symbols`: unrelated helpers, surviving exact entries, malformed/truncated records. Replacement tests mode-specific evidence and intentionally non-inlined binaries. |
| Fixed GCC staging/build route in [toolchain/build.c](../../src/codegen/toolchain/build.c) and `process.c` | Existing explicit low-level `--emit ... --compile` still has a live consumer. T253/T254 provide configure/build/verification; T255 migrates high-level orchestration. Remove the superseded fixed route and dedicated assumptions when its final required consumer migrates; retain only explicitly admitted low-level behavior with a real owner. | Current integration/assembly compile-and-run consumers. Replacement refuses failed stages/old-artifact execution and covers recovery/ownership. A passing legacy test alone cannot justify retention. |
| `src/library/artifact/` packaging/process/verification | Retain checked library/object consumers under EMBED-06; adapt shared verifier evidence in T254. Do not delete stateful library packaging merely because the one-shot application route changes. | Library artifact/execution/nested consumers with actual C toolchain runs. Retire a helper only after tracing its final library consumer. |
| [tools/project/process](../../tools/project/process/process.h) and [prepared test API](../../tools/project/testing/testing.h) | T233 owns current synchronous process/test primitives. T253/T255 adapt this separate tool boundary for build/run; T242 connects the accepted CMake test adapter. No compiler dispatcher dependency. | Project experiments/compiled/building leaves: exit/signal/exec distinction, exact argv, isolated Ama tests and private incremental builds. Replacement records profile/native closure and full build provenance. |

This ledger assigns replacement owners instead of preserving a second language
contract. Shared components retain named current consumers. Removal happens in
the same task as consumer migration; neither an unused implementation nor tests
dedicated solely to an abandoned behavior survive that commit.

## Typed handoff model

These names denote algebraic data boundaries. Concrete C declarations belong to
the listed implementation tasks and must cite this contract and canonical rules.
Every owner has one destructor that also accepts partial preparation. Borrowed
views expire before their source/IR/arena owner; no serialized pointer is an ID.

### Delivery request — T245

Under [CLI-01](../../spec/implementation/launch-and-embedding.md#cli-01) and
[EMBED-01](../../spec/implementation/c-library.md#embed-01) and
[EMBED-09](../../spec/implementation/c-library.md#embed-09), use:

```text
DeliveryRequest = {
  root: CanonicalModuleIdentity,
  component: CompleteComponentIdentity,
  mode: Executable | SourceInclusion,
  outputs: ExplicitOutputNamesAndOwnedDestination,
  target: CheckedTargetProfile,
  transports: ExactInputSchemaBindings
}
ResolvedDelivery = {
  request: DeliveryRequest,
  frontend: BorrowedValidFrontend,
  root_type: ClosedExecutableType,
  inputs: DeclaredInputTable,
  closure: ResolvedBuildClosure
}
```

Raw argv/configuration remains borrowed only during validation; validated request
strings/arrays are owned or explicitly borrow the longer-lived request owner.
The resolved view borrows the frontend/IR through generation. Root/mode/schema
failure publishes no resolved delivery and performs no output/native execution.
Runtime argument values are absent from this compilation identity. A runtime
string cannot close a template. Static/marker/open roots fail before emission.

Input IDs are the selected root's exact declaration identities/ordinals; names
retain their actual UTF-8 spelling. Descriptor ordinals are component-local and
cannot identify a Type across unrelated manifests. Nominal equality comes from
the checked semantic specialization identity, including its complete bindings,
not a C layout or human-readable basename. Finite resolved Type graphs encode
references to existing descriptor nodes rather than recursively expanding cycles.

### Native closure and authored instructions — T239/T247

Under [BUILD-01](../../spec/implementation/application-builds.md#build-01) and
[PKG-07](../../spec/project/packages.md#pkg-07), define:

```text
NativeRelation = {
  component_identity, resolved_module_identity,
  concrete_type_ids, owned_input_snapshots,
  c, h, abi, cmake: Present(bytes, origin) | Absent(origin),
  declared_resources, effective_version_root,
  readme: Present(path, bytes) | Absent(expected_path)
}
ResolvedBuildClosure = finite ordered NativeRelation table + dependency edges
```

Deduplicate the CMake application key `(component, resolved native module)`;
instance count does not multiply it. Use full logical module identities for the
stable relation order. Preserve CMake/link-target semantics after that order is
established: link options are not sets and may require repeats. Unused installed
modules contribute nothing. Required absence remains absence; an ignored package
copy cannot fill it. Captured text/provenance stays owned through emission and
build-record construction. A relative authored resource is checked within its
selected module/version contract before use; arbitrary assets are not guessed.

The compiler emits instructions/metadata but executes none. T247 applies each
participating authored function once to the supplied target, in the documented
order and a bounded local CMake scope. That scope does not isolate C macros,
packing or incompatible translation environments in one Ama assembly.

### Argument admission and construction — T240/T248/T249/T257

Under [CLI-01](../../spec/implementation/launch-and-embedding.md#cli-01),
[CLI-02](../../spec/implementation/launch-and-embedding.md#cli-02) and
[CLI-03](../../spec/implementation/launch-and-embedding.md#cli-03):

```text
AdmittedBindings = partial function InputID ->
  InlineText(BorrowedArgumentBytes) |
  TextFile(BorrowedPath) | BinaryFile(BorrowedPath, ExactDecoder)
ConstructionPlan = declaration-ordered InputID -> SuppliedRecipe | DeclaredDefault
```

Parse the entire token vector before file I/O, constructors or the root body.
The first `=` and exact suffix identify one binding; later equals and all payload
bytes remain data. Reject duplicate/unknown names, unsupported transports and
more than one `-` stream consumer. Ordinary C pointer readability is the caller's
precondition; malformed supported vectors and tokens are managed refusals.

Only after admission, initialize a fresh context/constants and construct inputs
in declaration order. Read/decode a supplied file immediately for that input;
earlier failure prevents later reads. Later defaults see the installed prefix.
Construct each input once using the chosen recipe, then results/state. An omitted
input uses its declared default, while explicit empty text invokes `|`. No
second interface reset follows fresh construction. Existing-root entry/resume
keeps its existing separate lifecycle contract.

Argument arrays live through the call, file buffers through their constructors,
and recipe metadata through construction. Successful Ama state independently
owns its values/resources. Close opened files once; borrow stdin and restore any
explicit mode change. Failure releases each successfully acquired prefix under
LIFE-05; it cannot roll back bytes already consumed from stdin.

### Checked binary decoder — T256

[CLI-03](../../spec/implementation/launch-and-embedding.md#cli-03) and
[EMBED-04](../../spec/implementation/c-library.md#embed-04) require this typed edge:

```text
ExactDecoder = {
  exact_nominal_type, checked_public_wire_schema,
  format_identity_and_edition, declared_byte_and_value_bounds,
  decoder_declaration_and_symbol, implementation_inputs,
  ownership_and_failure_contract
}
Decode : (BoundedImmutableBytes, DecodeOwner) ->
  Success(CheckedStagingValue<ExactPublicSchema>) | TechnicalFailure
Materialize : CheckedStagingValue<ExactPublicSchema> -> FreshAmaConstructionRecipe
```

The backend binds the concrete C function declaration/output wire type to the
selected checked schema and verifies it by compiling the declaration/adapter.
This is a family of exact typed adapters, not a layout-based universal cast.
The existing checked library transport supplies schema/shape/ownership checks
and fresh materialization where admitted. Unsupported schemas refuse generation;
absence never selects a decoder by filename. A binary input never passes through
UTF-8 conversion or `|`. The staging owner releases partial allocations on any
failure and survives until materialization finishes. No token/file pointer or
encoded address becomes a persistent Ama reference.

T256 defines/implements each concrete admitted schema adapter and its declared
format tests. T244 does not invent a universal on-disk value format or add a new
`.abi` capability. Decoder implementation and declared inputs participate in
closure identity and invalidation.

### One-shot operation and public entry — T250/T251/T241

Under [CLI-04](../../spec/implementation/launch-and-embedding.md#cli-04),
[EMBED-09](../../spec/implementation/c-library.md#embed-09) and
[RT-11](../../spec/implementation/runtime.md#rt-11):

```text
OneShot : (int argc, char **argv) -> int
  admission -> constants -> first root with supplied/default recipes
  -> one entry dispatch -> cleanup -> return
ExecutableMain = forward original argc/argv to the same OneShot
SourceInclusion = matching self-contained header + implementation with no main
```

Fresh context/pools/constant-lifecycle state belong to each call. No global
working instance, saved continuation, hidden scheduler or retained result escapes.
A yielded root is disposed without resumption. Ordinary result values do not
select a process status. The initial supported native POSIX profile allocates
status 0 to completed execution with successful cleanup and status 1 to every
unsuccessful one-shot outcome; typed diagnostics retain the distinct cause.
A different supported target must explicitly document its allocation.

Use the exact injective EMBED-09 component/path byte-hex public name, separately
from internal hashed identities. Detect target identifier limits; never truncate.
One implementation belongs to one host translation unit. Inclusion does not
add it again as a separate CMake source. Preserve the checked stateful C-library
API and its explicit contexts/resumption as a separate live consumer.

### Build profile, candidate and verification — T252/T253/T254

Under [BUILD-03](../../spec/implementation/application-builds.md#build-03) and
[BUILD-04](../../spec/implementation/application-builds.md#build-04):

```text
BuildProfile = resolved target/ABI/C/compiler/toolchain/CMake/configuration
  + required verification settings + declared provider/environment inputs
BuildIdentity = DeliveryIdentity + InputSnapshots + BuildProfile + NativeRequirements
Candidate = owned generated/build paths + actual artifacts + BuildIdentity
VerifiedCandidate = Candidate + exact mode-aware VerificationEvidence
PublishedArtifact = VerifiedCandidate + committed artifact/record paths
```

A documented host-default profile records its concrete resolved values; target
libraries never silently come from the host during cross compilation. One
executor owns each mutable output tree. Other jobs get separate trees or a
checked exclusive ownership protocol. Generation-only stops before native tools.

Configure, build, verify and publish are distinct state transitions; failure
cannot advance the candidate. The verifier consumes actual unstripped supported
object/artifact evidence, exact native/control identities and the selected entry
mode. Presence of `main`, an inline attribute or a successful linker is
insufficient. Unsupported profile/unknown verification evidence is a refusal.
Source-inclusion verification checks its actual host translation unit/target,
not an independently compiled duplicate implementation.

Reuse requires equal complete declared identities/records and actual build-system
dependency tracking. Provider inputs with no stable identity are explicitly
unknown and disable reuse. Timestamps/existing binaries are not proof. An old
publication remains an identified prior result after a failed request and is
never executed as that request's result. Authored CMake effects remain trusted
external effects; output cleanup does not promise their universal rollback.

### Stage result, diagnostics and cleanup — T255/T242

Under [BUILD-05](../../spec/implementation/application-builds.md#build-05):

```text
StageResult = {
  stage, established_native_substage?, component/test/input identity,
  process?: {exact argv, tool, exit | signal | launch_error},
  primary_diagnostic, original_output,
  proven_module_origins, relevant_instructions,
  effective_version_roots_and_readme_presence,
  secondary_cleanup_or_report_errors, retained_owned_paths
}
```

Track validation/emission, configuration, native build (with compile/link
substage when established), verification, publication, launch and running-program
outcomes separately. A build-engine failure without reliable substage/module
provenance remains attributed to that component/build stage and participating
instructions; diagnostic wording does not justify a guessed culprit or OS package.
Preserve the original output and primary failure even when cleanup/reporting
also fails. Do not fabricate README content/path presence or install libraries.

The explicit project process adapter owns each synchronous child and exact argv.
It already distinguishes launch error, exit and signal. T253/T255 add build
provenance/context; compiler code does not borrow its dispatcher. T242 runs each
selected test through that accepted adapter with a distinct output/context and
records outcomes in the T233 orchestration boundary.

## Concrete backend metadata envelopes

These are generated implementation metadata, outside the Ama source namespace.
Their initial encoding is strict UTF-8 JSON with a `format` string and explicit
edition 1. Parsers reject duplicate/unknown keys, missing required fields,
unknown enum values, mismatched referenced IDs, unsupported editions and size
arithmetic overflow. Paths and names are JSON data, never shell/CMake fragments.
Canonical Ama names/identities remain UTF-8 JSON strings. Host argument,
physical-path and environment bytes use `ByteString = {hex, length}`: hex is
exactly two lowercase hexadecimal digits per byte and length is the checked
byte count. This preserves non-UTF-8 host bytes without replacement. Path/argv
fields reject embedded zero bytes; bounded input data remains length-bearing.
Raw subprocess output stays in owned byte-preserving log artifacts; metadata
records their paths, lengths and content identities. Optional display text is
a presentation view, never the authoritative request/log value. A target that
cannot represent an admitted physical path diagnoses that limitation before
using it. Ordinals are
nonnegative checked integers, with nonzero descriptor IDs where the IR requires
one. Identical scalar names do not merge typed ID domains.

| Envelope | Required typed groups | Producer / consumer and owner |
|---|---|---|
| `ama.delivery.v1` | mode, root/component IDs, target/ABI edition, output paths, component-local Type/field graph, admitted transport declarations, input closure, native relations, exact verification entries | T245/T239 compiler emission; T252/T253 planner. Borrow IR only while constructing an owned serialized record. |
| `ama.binary-transport.v1` | exact Type/schema binding, format identity/edition, bounds, typed decoder declaration/symbol, implementation/dependency inputs, ownership policy identifier | T245 records exact input identity and declared literal capability; T256 parses/checks explicit binary-adapter metadata and emits its typed adapter. Incompatible wire/Type IDs fail before generation succeeds. |
| `ama.build-profile.v1` | target/toolchain/ABI/C settings, concrete CMake generator/configuration, verification policy, dependency/provider/environment inputs and editions | T252 resolved profile; T253/T254 actual commands/evidence. Runtime argv is excluded. |
| `ama.build-record.v1` | delivery/profile/input identities and contents/hashes, companion/resource presence, effective settings/providers, exact command/results, artifact IDs, verification, publication state, diagnostic/log paths | T252–T255 build owner; T242 test composition. Unknown provider effects cannot be replaced by invented hashes. |

These group names fix domain/ownership and version admission. T245/T239/T252/T256
own concrete record fields/codecs together with their actual producing consumers;
they must update this contract, not introduce a second metadata interpretation.
Cross-references identify records within the same validated component/manifest;
IDs alone cannot cross unrelated builds. Byte/content equality and checked
identity remain distinct from a hash lookup accelerator.

Generated CMake consumes owned validated metadata and produces argument vectors
or properly encoded CMake data. A serializer must prove round-trip preservation
for quotes, backslashes, semicolons, newlines, Unicode and delimiter-like bytes;
it must not concatenate user values into executable command fragments. Retain
link-item order/repetition after CMake establishes their meaning.

## Correctness arguments and acceptance partitions

1. Admission is a partial function over exact declared input identities. Each
   accepted token adds one previously unbound ID, so duplicates refuse before
   I/O. Parsing advances to the next finite argv element; the first-equals rule
   uniquely partitions each token because names exclude colon/equals.
2. Construction consumes the finite declaration sequence once. Inductively the
   installed prefix has exactly one chosen construction per input and is visible
   to the next initializer. Failure unwinds exactly that completed prefix, then
   remaining owned context resources. Native callbacks obey their existing
   failure/ownership contract; no rollback of consumed streams is claimed.
3. Closure/application keys are `(component, resolved module)`. Finite checked
   dependency traversal and exact set membership produce one application per
   key; distinct component targets remain distinct. No instance count or
   directory-enumeration choice influences the native application set.
4. Public name encoding uses two hex digits per byte and a separator outside
   the hex alphabet. Decoding is unique, hence component/path pairs are injective.
   Checked sums bound allocation; target-length refusal preserves injectivity.
5. Candidate stages form an acyclic order. Only verification success admits
   publication, and only that request's publication admits run. A failed edge
   cannot reach a prior successful artifact because the artifact/identity handle
   is carried explicitly, rather than rediscovered by filename.
6. Owners form an explicit release graph. Borrowed argv/streams/IR/views expire
   before their owners; prepared values and contexts retain no borrowed byte
   buffer. Cleanup preserves the first failure and collects later errors without
   retrying unknown destructive callbacks.

The bounded algorithms must report actual costs in their implementation tasks:
argv recognition is linear in token bytes plus declared-name lookup cost;
construction is linear in visited declarations plus constructors/file bytes;
closure traversal is finite in captured nodes/edges plus exact-key lookup cost;
serialization is linear in represented bytes/edges with graph references;
verification is linear in supported listing bytes plus exact-entry lookup cost.
External build/tool/runtime execution is not assigned a fictitious finite
algorithmic bound by these metadata proofs.

| Partition | Current consumer witness | Future acceptance owner |
|---|---|---|
| Exact commands/explicit outputs, invalid root/mode/schema and allocation refusal | driver request and codegen assembly leaves | T245 |
| Shared/specialized native closure, absence/empty/changed companions, ignored copies, resources/README | project inputs and source configuration/module leaves | T239/T247 |
| Author bytes/locks/concurrency/prepare/publish/rollback/cleanup failures | native scaffold publication leaves | T246 |
| Supplied/default order, exact once, prior-field defaults, constructor/OOM/destructor errors | program-construction and context leaves | T248/T249 |
| Empty/multiword/Unicode tokens, duplicate forms, later equals, null/count/sentinel shapes, no I/O before admission | library transport shape/recipe consumers | T240 |
| Files/stdin, embedded NUL/BOM/endings, partial reads, close/mode restoration and earlier-failure suppression | existing byte/source owners and explicit checked transport | T257 |
| Exact schema vs same layout, unconfigured/incorrect decoder, malformed/truncated/range/size/ownership failures | checked library transport + real native consumers | T256 |
| Repeated/concurrent fresh calls, no main in inclusion, both modes agree, yield/technical/cleanup outcomes | assembly/library execution/context/nested consumers | T250/T241 |
| Unicode/version/component names, delimiter vectors, overflow/identifier limit | current identity registry and library identities | T251 |
| Profile/closure/provider changes, unknown effects, changed presence, cache mismatch, cross-run without executor | input graph comparison and actual artifact consumers | T252 |
| Actual configure/compile/link failures, preserved caller cwd/argv, failed verification and no old-artifact run | real compiler/library/tool process consumers | T253/T254/T255 |
| Two real CMake components/manual host include and independent delivered tests | current library nested + isolated project compiled tests | T242 |

These are scoped witnesses and planned checks, not a claim that the new behavior
already passed. Later tasks add granular/component/composition/model/OOM/sanitizer
and compiled execution evidence with their exact source/input fingerprints.

## T244 acceptance and continuation

The current source consumers, their current tests and every replacement owner
were inspected read-only. Generation still emits the fixed four-artifact/main
route, startup still supplies no first-root input recipe, and verification still
requires main in its current object path. Those findings are migration inputs,
not reasons to change the specification or certify the newer behavior early.

T244 changes documents only. Required acceptance is link/owner/coverage consistency
and checked Doxygen generation; no C/runtime regression result is invented.
Next: validate the documented actual paths/test witnesses and specification IDs,
generate the final documentation, archive T244 and commit. T245 then implements
the typed delivery-request boundary before native-closure capture in T239.

### T244 verification record

Actual caller tracing covered request/options, the fixed project artifact loop,
host first-root startup, default program construction, checked library schema
and recipe emission, library identity registration, exact symbol verification
and both live process/artifact routes. All twelve named concrete witness leaves
have their maintained Makefile and main.c. All contract file links and explicit
canonical rule anchors resolve locally. This is read-only inventory/contract
evidence, not execution of proposed functionality.

No maintained non-Markdown input changed after the accepted `dc284fb` baseline.
Run the supported Doxygen builder at `/tmp/ama-t244-documentation.log` to check
new page/rule links and final input provenance. Existing per-file code/header
evidence remains unchanged; no unrelated runtime test rerun is required for this
documentation-only task. After a pass, archive T244, regenerate for Markdown-only
archival and commit.

### Final T244 acceptance

Doxygen builder 54170 exited zero at `/tmp/ama-t244-documentation.log`: 2522
maintained file pages, 9698 interfaces and 493 checked destinations. Manifest
and delivered-site input fingerprints agreed. Every working change is Markdown;
no C/header/build/test/schema implementation bytes changed after `dc284fb`.
No runtime regression execution or future component support is claimed.

T244 is archived in DONE. Regenerate Doxygen after this archival and verify
that all accepted non-Markdown inputs remain unchanged before the task commit.
At the T244 checkpoint, the next ready task was T245. Its subsequent acceptance is recorded below.

## T245 active implementation checkpoint

T245 starts from `63a2d36`. Actual emission has two consumers: the compiler's
application path and checked library packaging. The latter currently generates
then removes launcher/tooling artifacts by filename; migrate it to a shared
assembly boundary instead of attaching one-shot metadata to a stateful library.

The existing library binding validator requires every exported field and its
scalar profiles require clone/condition hooks. CLI admission has different
coverage/capability rules: defaulted complex inputs and unread results need no
codec, and owned scalar text needs the actual literal constructor. Reuse exact
nominal/ownership primitives where applicable; do not apply the full-export
coverage rule or extra clone requirements to launch inputs.

Planned small steps: (1) an owned explicit component/mode plus actual validated
executable-root selection, (2) exact declaration-ordered input/transport metadata
with omission preserved, (3) driver/export/shared-assembly integration and owned
metadata emission, (4) component/composition/OOM/schema refusal and no hidden
process/source mutation checks, registry and Doxygen. T241/T240/T256 retain
their actual launcher/token/decoder implementation responsibilities. Generation
must not publish mode-inconsistent artifacts while those consumers migrate.

Current step: implement/test the root/component selection child. No T245 checks
have run, no new behavior is accepted, and no job is active.

The first selection child is implemented under `src/delivery/selection/`.
`make -j2 -C tests/unit/delivery/selection` exited zero at
`/tmp/ama-t245-selection.log` (7569). It uses actual checked ordinary/native
frontends, both mode data values, copied Unicode/space-preserving component
identity, malformed UTF-8/unknown mode refusal, unchecked/static/open root
refusal and every measured selection allocation failure with retry. Native
fixture companions contain intentional `#error` directives, so these accepted
check-only tests cannot accidentally compile/execute native C.

This is a model child only: it is not yet integrated into driver/emission and
does not claim a new launcher. T245 remains active/uncommitted. No job is active.
Next: exact declaration-ordered input capability metadata, then actual owned
request/driver/shared-assembly integration and schema handling.

The declaration-ordered input child is implemented under `src/delivery/inputs/`.
Its first build failed on a diagnostic field spelling (`location` instead of the
actual `primary` member); the corrected complete leaf exited zero at
`/tmp/ama-t245-inputs-fixed.log`. It preserves default-only reference/list/compound
inputs, excludes results, admits a literal-only noncopyable native Type without
condition hooks, checks exact multiword names/parameter ordinals, refuses a live
invalidated frontend before old IR dereference, and recovers table OOM.
The failed command is `/tmp/ama-t245-inputs.log` (51824); it is not acceptance.

Current T245 files are working changes, not registered/accepted delivery support.
Next integration must separate shared assembly from application artifacts: the
stateful library build currently calls `project_codegen_build`, then removes
launcher/tooling names. Do not attach one-shot application metadata to that
library consumer. Add explicit compiler mode/component request handling and
wire the checked selection/table into actual emission; later launcher/decoder
tasks retain their documented implementation owners. All current jobs are
collected. No source or specification ambiguity is open.

<a id="t245-continuation"></a>

### T245 continuation after connection recovery

Confirmed that `/tmp/ama-t245-assembly-integration.log` reached successful test
execution and automatic cleanup. The standalone explicit delivery options suite
passed with required mode/component and refusal of omission, repetition, unknown
mode and executable compilation of source-inclusion mode:
`make -C tests/unit/driver/options` (`/tmp/ama-t245-options-boundaries.log`).
`make -j2 -C tests/unit/library/execution` exited zero
(`/tmp/ama-t245-library-assembly.log`, session 27790 collected). Stateful library
assembly now creates only shared declarations/body and no longer removes a
launcher after emission. Actual application tests use explicit delivery selection;
existing process-based emission consumers are being migrated to explicit flags.

`make -j2 -C tests/unit/codegen/assembly` passed with `delivery.json` staged as
an owned artifact (`/tmp/ama-t245-metadata-assembly.log`, 54175 collected), including
every measured allocation refusal. Added an independent JSON parser case for
exact field names/capabilities, complete framed nominal keys, graph-edge membership
and a recursive reference self edge; that new case still needs its next run.

Concrete initial delivery fields are `format`, `mode`, `component`, `root`,
`directory`, `types`, `inputs`, `generated` and `verification_entries`. IDs/ordinals and ByteString lengths are
unsigned decimal **strings** (not JSON floating-point numbers). Type nodes carry
component-local `id`, canonical `module`, complete framed `key` ByteString and
ordered `fields`; fields carry exact UTF-8 `name`, semantic `role` (`parameter`, `result`, `state`), role `ordinal`,
resolved `type`, `reference` and `list`. Inputs carry exact `name`, `ordinal`,
literal capability `text` and `binary` (currently null: no configured decoder is
claimed). The finite graph references IDs rather than expanding Type cycles.
T239 still owns companion/build closure additions. Runtime argument values are
absent. Source-inclusion metadata selection is supported by the selection child;
the application emitter refuses it before artifacts pending T241 packaging.

Remaining T245 work: finish actual metadata/schema admission and refusal coverage,
run new graph case and migrated real consumers, complete required ownership/failure
checks, reconcile current usage, register every maintained input and run Doxygen.
T245 is incomplete and uncommitted. No semantic ambiguity is open. No job is active
at this checkpoint; the next action is the metadata graph test followed by actual
driver emission and remaining schema boundary work. No specification files changed.

The independent finite graph JSON case passed in
`make -j2 -C tests/unit/codegen/assembly`
(`/tmp/ama-t245-metadata-graph.log`, session 11293 collected, exit zero).
The real command-line consumer compiled and ran the minimal application both
normally and with its existing generated-target ASan/UBSan configuration:
`make -j2 -C tests/integration/minimal`
(`/tmp/ama-t245-real-driver.log`, session 22338 collected, exit zero).
These verify actual driver-to-artifact-to-C execution, not runtime argument support.
Added pending cases for all 256 byte values/empty ByteString/output failure and
application-refusal versus shared-library isolation. Project init guidance now
includes explicit mode/component flags. T245 remains active; registry, remaining
schema contract and final acceptance are outstanding.

The boundary cases also passed with the full assembly suite
(`/tmp/ama-t245-metadata-boundaries.log`, session 92062 collected, exit zero):
all 256 byte values, empty record, `/dev/full` failure, unsupported packaging before
artifacts, shared-library isolation and the existing complete OOM sweep. New
maintained files and affected interfaces are now registered to named actual cases;
registry validation is still pending. No task completion is claimed.

Initial registry/interface coverage validation exited zero:
`python3 tools/documentation/coverage.py`
(`/tmp/ama-t245-coverage-initial.log`, session 37586 collected). This proves
registered file/header/case interfaces, not behavioral test execution.
The first delivery parent command failed before any leaf due to a wrong relative
include of `parent.mk` (`/tmp/ama-t245-delivery-composition.log`); corrected it to
`../../parent.mk`. Metadata Type edges now use the existing exact
`native_scaffold_type` lookup instead of maintaining a duplicate lookup loop.
Current README and initialization command shapes carry explicit delivery flags.
Next run: corrected delivery parent, followed by actual project scaffold/compiled
consumer acceptance and metadata regression for that lookup simplification.

The corrected delivery parent passed (`/tmp/ama-t245-delivery-parent-fixed.log`,
21184 collected). Project command/initialization/compiled consumers passed
(`/tmp/ama-t245-project-consumers.log`, 3773 collected). The delivery parent also
passed ASan/UBSan with the documented non-PIE flags and leak instrumentation
disabled (`/tmp/ama-t245-delivery-sanitized.log`, 55999 collected); counted arena
failure tests remain separate ownership evidence.

The four-leaf codegen consumer command failed overall
(`/tmp/ama-t245-codegen-consumers.log`, 59529 collected, exit two), first exposing
the old negative case that forbids structural inference through a known producer.
Read-only actual `ama --check` diagnosed the incomplete selection at main line 11,
with factory as related provenance (`/tmp/ama-t245-static-diagnostic.stderr.log`).
Inspection found a deeper conformance issue than the old location assertion:
TPL-10 step 5, TPL-15 and TYPE-01 now permit simultaneous structural selected-output
and ordinary-call equations. The T271/T226 implementation note's blanket ban on
consumer evidence is therefore obsolete. Do not merely change the expected
error span to make the old negative test pass.

The current correction equates a selected scalar to its original evaluated
structural definition before input closure; actual producer origin/definition are
already resolved. Complete-input, unused-input, graph/provenance and publication
checks remain independent admission obligations. No candidate search or invented
inverse is added. The old negative composition is now a generated-execution
positive for a uniquely determined nested producer result. This correction and
all related negative/cycle/OOM cases require acceptance before T245 can finish.
Current next run: specialization composition. T245 remains incomplete; no commit
or new conformance claim has been made, and no specification has changed.

The corrected real nested composition passed
`make -j2 -C tests/unit/codegen/specializations`
(`/tmp/ama-t245-structural-equation.log`, 50883 collected, exit zero), including
its existing full generation-allocation sweep. Other leaves of the failed
four-leaf run completed successfully (assembly, layouts and execution), but the
combined command remains failed evidence. Updated the direct selection test to
separate reflected-input inference from discarded-input underdetermination and
removed the unused context parameter from the scalar-equation interface.
The next acceptance is the direct result-selection and publication contracts.

Direct results `selection` and `cache` acceptance passed after the simultaneous
constraint correction (`make -j2 -C tests/unit/semantic/static/results selection cache`,
`/tmp/ama-t245-selected-constraints.log`, 7228 collected, exit zero).
Active behavioral run: `make -k -j2 -C tests/unit/semantic/static`, session **86759**,
log `/tmp/ama-t245-static-matrix.log`. Production/test inputs are frozen while it
runs; this continuation note is not a behavioral input. Collect its actual exit
and all failure leaves before modifying code. The corrected generated composition
passed earlier; the direct test distinguishes reflected and discarded inputs.
Next: resolve any matrix failures against canonical rules, register the changed
interfaces/cases, finish T245 transport/metadata and required final acceptance.
No task is complete, no new commit was made and no user clarification is pending.

Additional compiler consumer run collected: `make -k -j2 -C tests/unit/compiler
specialization-composition specialization-diagnostics`
(`/tmp/ama-t245-compiler-constraints.log`, session 76282, exit two).
Specialization composition passed. Diagnostics exposed a **real remaining
provenance regression**, not an obsolete semantic expectation: a factory's
selected output is fixed integer while another public output uses its missing
input. Rejection remains correct, but the error falls back to generic main:1:1,
losing both the exact selector and supplier definition witnesses required by the
existing diagnostic composition. Keep that assertion and repair pending-producer
obligation diagnostics; do not weaken it to accept a generic root message.

Static matrix 86759 is still active; 22 harness invocations observed with no
reported assertion yet. Do not edit production/tests until its completion. Next
investigation: retain explicit pending full-input/graph obligations even after the
selected scalar is known, and attribute their refusal to the actual selector and
original supplier. All other T245 acceptance remains outstanding.

The full static matrix completed all **97** harnesses (86759 collected, exit two).
Four leaves failed: annotations/ordinary/constants-initializer retained the old
blanket prohibition and now become positive structural-inference coverage;
closure/scheduler required an implementation-specific publication counter even
though the actual graph/key closure was established. The scheduler now checks
those real supplier graph/key proofs and the absence of a runtime producer entry.
No test is removed or weakened to accept an unresolved supplier.

Mandatory input closure is now recorded independently of scalar result closure
in `results/producers/requirements.c`, before publishing each prepared request.
Each actual input term becomes an existing solver requirement at the exact
selector (or static graph head). Omitted terms retain the original formal, so
underdetermination points both to the use and to the responsible supplier input.
All inputs remain mandatory, including ones discarded by selected outputs;
partial returned Templates remain complete metadata with known residuals.
No solve/body evaluation is performed during registration. The cached full map
is finite; repeated preparation reuses it and adds no duplicate requirements.
On allocation refusal, prior partial requirements remain arena-owned and no
request is published. The solver's original saturation/occurs/provenance checks
and the separate publication/executable gates remain authoritative.

Next run: the corrected four leaves and compiler diagnostics, followed by the
whole static acceptance for the new input-obligation registration. T245 remains
active; documentation/registry and remaining delivery acceptance are unfinished.
All earlier jobs are collected; no specification was changed.

The corrected focused commands all exited zero: publication `annotations ordinary`
(`/tmp/ama-t245-publication-corrected.log`, 56719), constants `initializer`
(`/tmp/ama-t245-constant-corrected.log`, 31876), closure `scheduler`
(`/tmp/ama-t245-scheduler-corrected.log`, 91792), and compiler
`specialization-diagnostics` (`/tmp/ama-t245-producer-provenance.log`, 17257).
All sessions are collected. In particular, the unchanged compiler diagnostic
expectations now retain exact supplier/use provenance, and the scheduler checks
actual closed supplier graphs instead of an incidental publication count.

The invariant is declarative: every actual producer input belongs to the solver's
required set independently of output projection. Structural result equations may
refine those inputs; they cannot remove a requirement for a discarded input.
Only final closure admits the result. The existing finite saturation includes
requirement-count changes and the existing exact term interning/cached maps avoid
fabricated progress. Requirements preserve original scoped formals and require no
new failure-time allocation. Byte/Type metadata uses existing registered identities
and exact lookup; field roles are stable explicit strings, not compiler enum values.

Next: repeat the whole static matrix after obligation registration, run the actual
metadata consumer after role serialization, register all changed/new interfaces,
then complete delivery-specific acceptance and Doxygen. T245 remains active.

Active continuation checkpoint (code/test inputs frozen):

- Full 97-leaf static repeat: `make -k -j2 -C tests/unit/semantic/static`,
  session **92355**, `/tmp/ama-t245-static-obligations-final.log`.
- Actual metadata/assembly consumer: `make -j2 -C tests/unit/codegen/assembly`,
  session **41567**, `/tmp/ama-t245-metadata-roles-final.log`.
- Sanitized compiler diagnostics: session **64428**,
  `/tmp/ama-t245-provenance-sanitized.log`; ASan/UBSan, `detect_leaks=0`,
  `halt_on_error=1`, the recorded non-PIE C/link flags. Counted OOM tests remain
  separate evidence; do not claim leak instrumentation.

Collect all three outcomes before code changes. After that, remaining T245 work
includes the delivery/transport ownership contract, final real consumers,
registry/interface validation and generated documentation. Binary byte-adapter
implementation and actual launcher packaging/token handling still have their
explicit T256/T241/T240 owners. No task is complete or committed yet.

Collected checks after the role/obligation corrections:

- Metadata/assembly **41567** exited zero
  (`/tmp/ama-t245-metadata-roles-final.log`).
- Compiler diagnostic ASan/UBSan **64428** exited zero
  (`/tmp/ama-t245-provenance-sanitized.log`); leak instrumentation was disabled.
- Registry/header/case-interface coverage **45440** exited zero:
  **3363 ready inputs, zero explicit debts**
  (`/tmp/ama-t245-coverage-constraints.log`). This is interface evidence, not a
  substitute for behavioral execution. Subsequently only purpose text and a named
  existing OOM-case registration were refined in ready.json.
- Real stateful CLI library builds and their failure/publication controls **96407**
  exited zero (`make -j2 -C tests/integration/library-build`,
  `/tmp/ama-t245-stateful-library-build.log`). Shared-assembly separation therefore
  passes the actual library build command, not only a manually linked host fixture.

Only **92355** remains active: the full static repeat at
`/tmp/ama-t245-static-obligations-final.log`. Collect it before production/test
changes. The metadata and diagnostic inputs are frozen. Future binary adapter
admission/token parsing/packaging remain their explicit roadmap owners; no
configured binary decoder or source-inclusion artifact is claimed by T245's
currently accepted children. T245 is not complete or committed.

**Full static acceptance collected:** session 92355 exited zero after all **97**
assertion harnesses (`/tmp/ama-t245-static-obligations-final.log`). No failed leaf
or assertion was reported. This accepts the simultaneous selected-output equations,
mandatory full-input obligations, actual graph checks and all existing cycle,
constant, ownership and allocation-refusal compositions. No static behavior/code
changed after this run; remaining changes concern delivery artifact selection.

Application delivery now retains the actual staged `entry_artifact` in its owner.
Tooling argv and private object compilation consume that record rather than
reconstructing a launcher filename. Stateful shared assembly has no entry record.
Delivery metadata adds `generated` (preceding owned artifacts' exact path bytes,
byte lengths and explicit entry marker) and `verification_entries` (actual C unit
and explicit main requirement from the selected delivery mode). The metadata file
is staged last and does not list itself; a subsequently compiled executable belongs
to the build result, not the generated-source list. This avoids recursive length
claims and filename-based compile-entry discovery. T239 still owns source/native
closure provenance; T252–T255 own actual build/profile/result records.

Pending: actual metadata/compilation consumer tests for this explicit unit wiring,
registry and Doxygen, then final T245 acceptance. Earlier static acceptance is
retained; no new compiler inference or language rule is introduced by this step.

### T245 admitted transport and consumer boundary

This foundation uses the existing checked frontend/IR and native ABI declarations.
For each root parameter it retains the original symbol (hence complete nominal
Type, storage shape and constructor declaration), its declaration ordinal and
literal capability. An owned scalar native-backed Type with `|` is the admitted
text profile. References, lists and compound Types remain in the table as
**default-only**, not generation errors: omission is legal under CLI-02.
Neither cloning nor a condition hook is an extra literal requirement. No result
codec is required merely because the root returns that result.

`binary: null` is an explicit absence of an admitted binary adapter, not a request
to discover one or deserialize payload memory. T256 already owns CLI-03/EMBED-04's
exact Type/schema/declaration admission together with the concrete decoder; its
first step also owns parsing the versioned binary-adapter descriptor. This
clarifies the duplicated ownership sentence in the T244 ledger. No binary
configuration is accepted unchecked, no required language behavior is removed,
and this component does not claim runtime token/file/decoder support. T240,
T248/T249, T257 and T256 remain the actual token, startup, text, file and binary
consumers respectively. T241/T251 own source-inclusion packaging and public names;
the current application emitter refuses that mode before staging any artifact.

A future metadata reader must validate the exact edition/closed field sets and
all ByteStrings/decimal strings before using paths or allocating arrays. Type IDs
are nonzero, unique and component-local; the root and every field edge refer to
that same graph. Roles are exactly parameter/result/state; original names and
per-role ordinals identify fields without normalization. Each input matches the
root's parameter name/ordinal and retains its actual reference/list flags through
the field graph. An admitted text profile cannot refer to reference/list storage.
Exactly one `generated` record is the explicit entry, and every verification
entry references an actual generated unit with the mode's main requirement.
The generated-source manifest does not assert that an executable was compiled;
that result belongs to the later build record. A consumer must not infer a C
unit by extension, a transport from a Type basename, or successful verification
from manifest presence. Runtime argument values never enter these identities.

Explicit-output acceptance `make -j2 -C tests/unit/codegen/assembly` passed
(`/tmp/ama-t245-explicit-output-records.log`, 97573 collected, exit zero).
The independent parser checks real published byte lengths, exactly one entry,
verification membership and equality with compilation database argv/file selection.
The existing every-allocation failure sweep also passed. The earlier complete
97-harness static acceptance remains valid; these changes affect only delivery
artifact metadata/tooling selection, not static equations or closure.

Actual executable delivery after explicit entry wiring passed the real CLI:
`make -j2 -C tests/integration/minimal`
(`/tmp/ama-t245-explicit-entry-driver.log`, 63082 collected, exit zero), including
its ordinary and generated-target ASan/UBSan execution. The final specialization
codegen suite also passed (`/tmp/ama-t245-specialized-delivery-final.log`, 79802
collected, exit zero), including nested inferred composition and generation OOM.
Only interface/purpose comments changed after those runs.

Review of the driver consumer's explicit link inputs found the shared JSON writer
listed both through scaffold dependencies and the driver's whole output subtree.
The driver list now excludes that already owned child explicitly, instead of
silently sorting/deduplicating arbitrary link inputs. The real driver-run harness
will verify that composition and the help text's required mode/component flags.
No behavior/specification alternative is introduced by this build dependency fix.

### T245 final behavioral review before generated documentation

The actual driver composition passed after the explicit link-owner correction:
`make -j2 -C tests/unit/driver/run`
(`/tmp/ama-t245-driver-composition-final.log`, 64264 collected, exit zero).
Its help observation now requires the explicit `--mode MODE --component ID`
command shape. No arbitrary source-list deduplication or compatibility branch was
introduced. No behavioral job is active.

Current accepted boundaries are owned selection/input descriptors, explicit
application mode/component options, finite exact delivery JSON with actual output
and verification-unit records, shared stateful assembly without discarded launcher
artifacts, and the existing real executable toolchain consuming that same selected
unit. Source-inclusion artifacts, runtime token/file/startup adapters, explicit
binary decoder admission and CMake orchestration retain their named future owners.
This is a foundation acceptance, not a claim that those commands already work.

The static correctness repair is accepted by the complete 97-harness matrix,
positive generated nested composition, unchanged lost-input diagnostics and
ASan/UBSan compiler diagnostics. Every input remains a solver obligation; a known
output cannot erase a discarded input. The exact structural solver and creation/
value-cycle guards remain the only algorithms. Current input/entry metadata uses
that shared checked result rather than another resolver.

Remaining gate: `make docs` over the final maintained inputs, inspect final diff,
archive the full T245 record only after acceptance, regenerate after Markdown-only
archival and verify provenance/staged bytes before committing. The next ready
implementation task is T239. No open ambiguity or specification edit is pending.

The first final Doxygen command failed its rendered-link gate, after coverage
succeeded for 3364 inputs (`/tmp/ama-t245-documentation-final.log`, 26261 collected,
exit two). New input/selection/scaffold/options type/member contracts cite rules
whose descriptions render on class pages, so their owning file purposes now also
explain and link those same lifetime/transport/entry rules. Added an explicit
lifetime paragraph for the private flexible-array table owner as well. These are
comment-only corrections; no behavior, requirement or accepted test changes.
Next rerun: the complete supported `make docs` gate.

### T245 acceptance and continuation

`make docs` passed after the comment-only origin-page corrections
(`/tmp/ama-t245-documentation-links-fixed.log`, session 57727 collected, exit zero):
3364 ready inputs, zero debts, 2539 maintained file pages, 9749 documented interfaces,
493 validated destinations and 8020 total pages with Doxygen 1.11.0.
The generated manifest/report matched byte for byte. Accepted non-Markdown task
inputs are frozen in `/tmp/ama-t245-accepted-task-inputs.json`; the accepted full
manifest is `/tmp/ama-t245-accepted-manifest.json`. These temporary audit files are
supporting provenance, not the sole task record.

T245 is accepted in [DONE](../../DONE.md#t245). Its detailed behavior, deliberate
remaining owners, failed initial checks and corrected successful runs are retained
above. Only Markdown task archival/status reconciliation follows the accepted
source/interface gate. Regenerate the supported Doxygen output for those final
pages, verify every final input fingerprint and inspect staged task bytes before
committing. No behavioral job is active, no semantic ambiguity is open and no
specification or conformance fingerprint has changed. Continue with T239 after
this task's commit; completion of this foundation does not complete the sprint.
