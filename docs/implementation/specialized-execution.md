# Generated execution of concrete specializations

T188 follows [T187](../../DONE.md#t187) concrete planning. Canonical rules:
[GEN-01](../../spec/implementation/compiler.md#gen-01),
[GEN-02](../../spec/implementation/compiler.md#gen-02),
[STACK-04](../../spec/implementation/compiler.md#stack-04) and
[TPL-05](../../spec/language/templates.md#tpl-05). No specification change is
authorized. [T186](specialized-constants.md) retains the first observed duplicate
ordinary C type descriptions for distinct captures of one source module.

## Steps

1. Reproduce that failure through actual static exports, concrete constants,
   independent ordinary instances and native result observations. Preserve the
   failed compiler receipt. Correct the full-owner type/member naming boundary
   with the canonical structural codec, sharing it with constant naming rather
   than adding another path/string/digest interpretation.
2. Exercise nested/partial composition, equal-key code reuse with independent
   mutable instance state, retained interfaces, busy/refusal behavior, references,
   yields and ordinary versus explicit continuation copies. Verify one assembly,
   labeled Ama transfer and checked inlined native entry contracts.
3. Check malformed or incomplete inputs, exact identity collisions, allocation
   refusal and native startup/cleanup failures through model and generated execution.
   Run affected existing consumers, sanitizers and complete Doxygen/per-file checks;
   archive/commit only after acceptance.

## Initial boundary

The retained semantic and resolved plan graphs already distinguish concrete owner
pointers. T186 constant names use complete owner substitutions. Native scaffold
view/member/prefix names in `native/scaffold/model/types.c` still register only
original source path plus role/member, so different ordinary captures collide
before C compilation. C types and source provenance must remain distinct without
changing concrete native leaf ABI names or inventing suffixes. Equal owners must
share descriptions/code while declared instances remain independent pool objects.

The first step added an actual compiled regression before changing production code.

The initial strict O2/Werror regression ran in session 64533,
`/tmp/ama-t188-generated-regression-before.log`. It combines two captured constant
namespaces with concrete ordinary calls, a third independent equal-key instance,
and native observers of both concrete output types. An unused partial export
remains static. Its failed result was collected before modifying production inputs.

## Reproduced defect and active repair

The strict regression completed with exit 2. Its C compiler reported repeated
struct definitions from the two `.container` variants, preserving the T186
finding through actual calls and native result observers. The receipt above is
a real failed compiled composition, not an accepted limitation of the language.

`semantic/specializations/identity/key` now owns common structural owner
qualification, reused by constant member keys and native scaffold names. The
old embedded static-codec block in the constant member component was removed;
its original member validation/append operation remains in use. The old
path-only scaffold identity function was removed and replaced by one narrow
registry operation using the complete owner key plus a separately framed role.
Temporary descriptors are destroyed only after registry encoding owns the bytes.
Concrete zero-input category/role keys retain their exact canonical shape.

Scaffold ordering preserves original logical-path order and compares complete
encoded owner keys within one source definition. Digests are not comparison keys,
and discovery order does not decide captured-owner order. No runtime storage,
cache state, working-state mirror or new native template ABI was introduced.

The first strict repair receipt is `/tmp/ama-t188-full-owner-after.log`
(session 89041): it refused native-category use for captured ordinary owners,
as detailed below. No acceptance was claimed for that failed attempt.

The first repair correctly retained owner components but attempted the native
namespace category for ordinary captures. The encoder correctly refused that
shape (`/tmp/ama-t188-full-owner-build-diagnostic.log`); a temporary local trace
identified the category restriction and was removed immediately. NAME-05 reserves
AMA_NATIVE contexts for native companions, and TPL-13 keeps their definitions
concrete. Ordinary model prefixes therefore use complete Type keys; concrete
native leaves retain the exact original native namespace key. No codec restriction
or language requirement was weakened. The generated composition then passed:
`/tmp/ama-t188-native-leaf-categories.log`, strict O2/Werror.

The first four owner/registry component cases passed
(`/tmp/ama-t188-owner-components.log`), with all 3 structural-owner allocation
positions and 6/7 no-role/role registration positions refused independently.
They verify exact descriptors, equal-key reuse, distinct roles/captures, untouched
receivers, foreign/null/incomplete/aliased input refusals, native leaf identity
and preserved first diagnostics. A fifth explicit category-refusal case is now
included and passed; expanded generated and sanitizer acceptance is recorded below.

The two-capture continuation/reference scenario passed before the next fixture
was refused (`/tmp/ama-t188-continuations-composition.log`). The failed nested
fixture incorrectly expected ordinary call arguments to infer a static producer's
missing input. TPL-12 explicitly requires supplying static specialization inputs
independently of ordinary declaration/call constraints. Source diagnostics were
added to the shared assertion fixture to preserve that failure evidence:
`/tmp/ama-t188-composition-source-diagnostic.log`. The positive example now uses
an explicit closed `.assembly` binding; the original form is retained as an
independent negative frontend test of this exact restriction. No compiler rule or
inference algorithm was changed to accept the ill-formed example.

## Expanded scoped outcomes and active runs

The corrected closed nested assembly and the explicit forbidden-inference
frontend case passed (`/tmp/ama-t188-closed-composition.log`). The complete eight-
case generated suite then passed strict O2/Werror:
`/tmp/ama-t188-named-result-and-refusals.log`. Its first partial-family fixture
mixed named ordinary inputs with a positional output and was correctly refused
(`/tmp/ama-t188-expanded-execution-and-refusals.log`); the corrected call names
its output as well. This test correction did not change ordinary call syntax.

The accepted suite executes two distinct captured types and repeated equal-key
instances, four captured shared values, native observers, reference-based resume,
explicit continuation snapshots versus ordinary copies, nested static outputs,
a determined partial Template passed to another static producer, and same-key
nominal self-reference Busy refusal. Independent private native call counters
verify that shared code does not merge instance state. All 1398 allocator request
positions on the complete successful in-memory generation trace were refused
independently; each partial scaffold remains not-ready and destroyable and
restores the exact live-owner baseline. No failed scaffold is published, and
its admitted semantic and resolved-plan owners remain valid.

## Ownership, invariant and cost argument

The owner key belongs to an actual relation-checked semantic vertex. Pointer
membership is checked before following a caller-supplied owner; complete closure
is required before encoding captured bindings. The existing structural codec
retains original constructor/formal identities, categories and normalized bindings.
No display-string parsing, textual substitution, digest-as-equality or invented
suffix supplies identity. A role/member adds one independently length-framed
component. Consequently equality requires the same complete owner and same
category/role; unequal captures or roles cannot silently share a registered name.

Native namespace category remains restricted to concrete companion identities.
TPL-13 supplies concrete `@C` leaves; NAME-05's original no-member category/key
therefore remains unchanged for them. Ordinary view/prefix identities belong to
Type categories, so captured ordinary owners do not masquerade as native modules.
All temporary descriptor/component arrays expire after registration owns encoded
bytes. Source spans and original module paths remain borrowed from the frontend,
which outlives scaffold destruction. Failure changes no output key, publishes no
ready scaffold and leaves partial allocations with their explicit arena owners.
No runtime instance state enters the specialization or identity registries.

Let V be semantic vertices, R names already registered, B complete encoded key
bytes, and K/S the existing codec's time/scratch bounds. Owner membership costs
O(V), owner preparation O(K), registration O(B + R) including SHA-256 and exact
comparison after a matching digest. Optional role append copies the codec's zero
or two owner components, with checked count arithmetic. Temporary space is
O(S + B). Across Q registrations the registry arena may retain O(QB + R) storage,
including repeated encodings; name deduplication does not imply allocation-free
lookup. Model insertion uses O(N²) comparisons over N type records, each comparing
at most the original path and full owner-key lengths. It allocates no sort array.
The tuple of logical path and complete encoding is a total order over admitted
unique nominal owners; it is independent of discovery order and hash values.

Runtime transfer, pool identity and clone algorithms are unchanged. The actual
compiled cases observe independent native counters/address-sensitive payloads,
separate retained values and explicit resume positions under shared descriptors.
This composes the existing stable-slot, labeled one-assembly backend; no ordinary
Ama C-call recursion, indirect dispatcher or working-state save/restore is added.

## Scoped and aggregate acceptance receipts

Strict expanded generated suite:
`/tmp/ama-t188-named-result-and-refusals.log`; ASan/UBSan counterpart:
`/tmp/ama-t188-expanded-execution-sanitizers.log`, all eight cases and all 1398
successful-trace generation allocation refusals. Owner/role components:
`/tmp/ama-t188-owner-components.log`, then the complete five-case ASan/UBSan run
`/tmp/ama-t188-owner-components-sanitizers.log`, with all 3/6/7 allocation positions.
The complete native scaffold parent passed strict O2/Werror:
`/tmp/ama-t188-native-scaffold-parent.log`. The production compiler build passed:
`/tmp/ama-t188-compiler-build.log`. Counted fixture teardown is independent
resource evidence; LeakSanitizer is unavailable.

Strict runs use `C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`.
Sanitizer runs use O1 and the same strict warnings with
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`,
`LINKER_FLAGS='-fsanitize=address,undefined -no-pie'`,
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1. Leaf commands are
`make -C tests/unit/codegen/specializations` and
`make -C tests/unit/native/scaffold/specializations`.

Full workspace `make -k -j4 test` at those strict O2 flags completed with exit 2
solely from the pre-existing T196 conformance specification-review refusal.
Other independent branches passed, including current codegen, native, library,
editor, relocation and integration consumers. All 3218 frozen maintained inputs
matched after completion. Exact argv/PIDs/status and byte evidence are retained
in `/tmp/ama-t188-whole-project-job.json`,
`/tmp/ama-t188-whole-project-inputs.json`; run receipt:
`/tmp/ama-t188-whole-project.log`. No conformance fingerprints were refreshed.
This result is not a green whole-product conformance claim.

Complete per-file/header coverage receipt: `/tmp/ama-t188-docs-check.log`
(2971 ready inputs, zero debt). Initial Doxygen rendering passed in
`/tmp/ama-t188-docs-render.log`, checking 2261 maintained file pages, 8800 interface
descriptions and all 442 canonical destinations. The final exact archived inputs
are rendered separately: `/tmp/ama-t188-archive-docs.log`. Reference pages are
produced only by Doxygen. Full task/evidence record: [T188](../../DONE.md#t188).
T189 is the next ready implementation owner.

The first archive render refused two unquoted `@C` markers as Doxygen commands
(`/tmp/ama-t188-archive-docs-unquoted-marker.log`). Quoting the language marker
as inline code fixes the prose; executable inputs remain identical to the
accepted aggregate. The corrected final receipt uses the archive path above.
