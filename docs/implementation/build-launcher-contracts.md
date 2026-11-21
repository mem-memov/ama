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
| `DriverOptions` in [options.h](../../src/driver/options/options.h), exact request recognition in [request.h](../../src/driver/request/request.h), emission through [generate.c](../../src/codegen/project/generate.c) | T245 adapts the validated request; root/mode/component/transports become explicit before IR emission. T240 separately owns runtime token admission. Keep exact checking/library/editor requests with their current consumers. | `tests/unit/driver/request` exact-command cases; `tests/unit/codegen/assembly` real emission and allocation refusal. Replacement adds both output modes and malformed request/schema refusals. |
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
| `ama.binary-transport.v1` | exact Type/schema binding, format identity/edition, bounds, typed decoder declaration/symbol, implementation/dependency inputs, ownership policy identifier | T245 parses admission metadata; T256 checks/emits the actual typed adapter. Incompatible wire/Type IDs fail before generation succeeds. |
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
Next ready task: T245, typed delivery request and emission metadata.
