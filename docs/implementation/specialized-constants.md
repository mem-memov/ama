# Specialization-scoped constant lifecycle

T186 follows [T185 ordinary admission](specialized-semantics.md), committed as
0281859. Requirements are [CONST-02](../../spec/language/declarations.md#const-02),
[CONST-06](../../spec/language/declarations.md#const-06) and
[LIFE-03](../../spec/implementation/native-interface.md#life-03).
No specification change is authorized or needed.

## Steps and acceptance partitions

1. Verify each required full owner/member identity survives semantic constant
   graphs, startup plans, resolved shared storage and generated names. Reuse
   T185's complete owner graph keys. Add concrete composition tests for repeated
   equal substitutions, distinct substitutions, two members per owner, typed
   empty lists, literal/default/copied values and self-typed constants.
2. Exercise the real runtime declaration/lifetime consumer with these concrete
   plans. Verify one immutable table per owner in each context, independent
   tables across contexts, root startup after constants, reverse successful
   cleanup and native partial-construction behavior. Include every allocation
   position and controlled native failures, without executing Ama bodies to
   manufacture constant values.
3. Correct actual stale identity/lifecycle consumers, removing replaced code and
   sole-purpose tests. Run component/composition, sanitizer and documentation
   checks. Archive/commit only after scoped acceptance. T188 completes broader
   specialized generated-project startup acceptance.

## Initial findings and contract

Semantic constants already have separate SemanticSymbol pointers per closed
owner. Dependency vertices now use T185's retained full owner name and member
ordinal. Startup plans borrow those symbols and resolved shared storage borrows
those plans; their owner graphs outlive runtime tables.

The generated-name boundary in codegen/model/identity.c still registers only
original module path plus member. Distinct captures with equal member spelling
therefore had the same generated identity. The initial regression build
refused linking because the new test leaf omitted the actual native-toolchain
sources (`/tmp/ama-t186-constant-name-before.log`). After correcting its declared
dependencies, the regression failed the pairwise distinct-name assertion
(`/tmp/ama-t186-constant-name-regression.log`). This is the observed defect;
compiler requirements and expected names were not changed to fit the output.
Do not invent suffixes, use hashes as equality, or merge owners with equal values.
Zero-input concrete names must retain their existing complete identity.

The key is (complete owner identity, original shared member identity).
Key framing must remain injective: distinct owners or members cannot share a
registered key; equivalent complete owners and one member must reuse it. Naming
owns copied key bytes, while source spans and semantic/plan objects stay borrowed.
Failures leave every initialized owner destroyable and do not publish runtime
state. Startup/cleanup remain context operations, never Type-cache mutations.

## Implemented key construction

`semantic/specializations/members/key` constructs
one constant-category qualified key from the complete canonical owner key plus
one independently framed original member. The replaced path-only naming logic
has been removed from `codegen/model/identity.c`; no compatibility fallback is
retained. The registry copies encoded bytes before temporary key-arena teardown.
Never parse explanatory `source_name` text or pass it as a module path.

The append is injective because each component has independent length framing:
the same complete owner and original member produces the same key; differing
owner descriptors or members produce different keys. The finite original-member
scan verifies pointer membership before following caller-supplied declaration
payloads. Owner traversal and member matching take O(T + F); codec cost is the
existing finite static-key plan cost. Append takes O(K + M) time and space for K
owner components and M member bytes, with checked count/byte arithmetic. Failed
preparation leaves output unchanged and partial allocations arena-owned. No
runtime table or specialization cache is mutated by key construction.

The corrected strict O2 leaf passed (`/tmp/ama-t186-constant-name-after.log`).
Every one of eight allocation request positions in the real scaffold naming
composition was then refused independently with balanced teardown and preserved
first diagnostics (`/tmp/ama-t186-constant-name-prefixes.log`). The strengthened
leaf also passed (`/tmp/ama-t186-key-structure.log`): it directly verifies three
components for captured owners, exact original member bytes, descriptor equality
iff owners match, and the unchanged single-member concrete key.

## Accepted execution and failure partitions

Generated execution passed strict O2 with literal/copy values and isolated context
storage (`/tmp/ama-t186-context-execution-single-owner.log`), then with all four
shared forms: literal, copy, native default and typed empty list
(`/tmp/ama-t186-context-all-value-forms.log`). Final ASan/UBSan execution additionally
mutates a private copy to 99, observes the copied sibling still at 17, and inspects
unchanged shared originals in each of two separately leased contexts:
`/tmp/ama-t186-final-context-copy-sanitizers.log`. Each context's worker starts
its own call state. A deliberately invalid specialized shared literal prevents
both root and namespace-body execution. The exact stdout excludes any accidental
namespace-body call.

Corrected test-source failures are retained: the first launcher initializer
triggered -Werror missing-braces; a direct body literal used in the first mutation
attempt was correctly rejected by the language, so the final fixture uses an
ordinary declared replacement value and assignment. These are test corrections,
not changes to language behavior or relaxed compiler flags.

Actual frontend checks distinguish an empty self-typed list from a real cycle of
cross-module shared-value reads. Strict O2 and ASan/UBSan receipts are
`/tmp/ama-t186-constant-cycles.log` and
`/tmp/ama-t186-keys-cycles-sanitizers.log`. The latter also covers direct key
shape, invalid receiver/membership/status refusals and every eight naming
allocation request positions with balanced teardown.

Runtime shared startup checks inject all four native construction positions.
The failed constructor cleans its own acquired resource; only earlier successful
values receive destructor calls, in reverse order. Each of two destructor
positions is refused both with and without a pre-existing primary failure:
cleanup continues, the primary survives, and another stop does not destroy twice.
Final ASan/UBSan receipt: `/tmp/ama-t186-final-runtime-sanitizers.log`; earlier
strict prefix receipt: `/tmp/ama-t186-runtime-prefix.log`. Counted fixture resource
balance is separate evidence; LeakSanitizer is unavailable.

Existing generated constant/state reads and assembly passed strict O2/Werror:
`/tmp/ama-t186-existing-constant-reads.log`,
`/tmp/ama-t186-existing-assembly.log`. The root production compiler is built using
`make -j4 C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`;
receipt: `/tmp/ama-t186-compiler-build.log`.
Sanitizer commands use the same strict warnings at O1 with address/undefined
sanitizers, frame pointers and non-PIE linking, ASAN_OPTIONS=detect_leaks=0:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1. Tests run in this workspace against the production
frontend, runtime and actual generated C toolchain; no replacement interpreter
or handwritten parallel specialization implementation supplies expected values.

## Remaining emission boundary and acceptance limits

An attempted two-capture generated composition exposed the existing
`native/scaffold/model/types.c` path-only ordinary type naming boundary:
`/tmp/ama-t186-context-execution-initializers.log` reports duplicate reference,
layout and debugger descriptions for `.container`. T188 already owns complete
per-specialization code/layout emission; its TODO records this exact case.
T186 does not claim that two-capture whole-project emission works. Distinct
captures are checked through actual frontend/storage and full constant keys;
accepted generated execution checks one captured owner in two contexts.
No failed case is reported as accepted, and no alternative naming contract or
specification change is introduced to avoid the observed limitation.

The known aggregate conformance-review refusal remains owned by T196; its
fingerprints were not refreshed. This is scoped lifecycle acceptance, not a green
whole-product conformance claim. Full task/evidence record: [T186](../../DONE.md#t186).
Complete per-file/header checks and Doxygen generation validate documented
interfaces and originating-file links to canonical rules. Initial receipt:
`/tmp/ama-t186-docs.log`; final exact archive receipt:
`/tmp/ama-t186-archive-docs.log`. Reference pages are generated only by Doxygen.
T187 is the next implementation task.
