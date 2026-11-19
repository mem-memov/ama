# Concrete native types in Ama template compositions

T190 follows T207, T212, T185, T187 and T188. Its rules are
[TPL-13](../../spec/language/templates.md#tpl-13),
[CABI-01](../../spec/implementation/native-interface.md#cabi-01),
[CABI-05](../../spec/implementation/native-interface.md#cabi-05) and
[CABI-06](../../spec/implementation/native-interface.md#cabi-06).
No specification change is authorized or needed.

## Boundary and planned checks

1. Instantiate an ordinary Ama algorithm with an actual concrete native Type.
   Compile and run literal/default/clone/condition/index operations after
   substitution. Refuse each missing required capability through the real
   frontend; a template application must not invent native hooks.
2. Select independently closed static exports for native owned/reference/list
   interface fields. Check resolved field identity, runtime-only ABI ordinals,
   typed views, native zero-own-formal admission and authored source preservation.
   Compile generated headers independently and execute the actual composition.
3. Check native lifecycle/layout failures and open selected dependencies; retain
   first errors and previous outputs, run relevant parent/consumer checks,
   sanitizers and documentation gates, then archive and commit separately.

The existing execution fixture owns authored test native companions and supplies
address-sensitive default/clone/destroy hooks. New cases may reuse this concrete
protocol, but must not add generic native authoring APIs or retain Ama references
inside opaque payloads. Original static producer completeness remains independent
of ordinary caller constraints. T194 owns foreign-child host cleanup acceptance.

## Algorithm and ownership argument

A native definition is admitted only with zero own formals. Ordinary template
substitution selects an existing concrete semantic Type; capability checks read
that Type's actual ABI bindings. Static export selection must close its producer
before admitting the selected Type. Neither operation adds runtime fields or
native function parameters. Existing resolved plans and generated typed adapters
remain authoritative; tests compare original field identity and explicit runtime
ordinals instead of parsing generated names or inferring capabilities from them.
Source/frontend/scaffold lifetimes follow their existing ownership order, and
all test-owned artifacts are isolated per leaf.

Current step: construct the independent compiled capability composition and its
refusal partitions. No production change has yet been justified. Existing native
own-formal admission cases are prerequisite evidence, not substitutes for these
actual compiled compositions. No active jobs.

First capability composition passed strict O2/Werror:
`/tmp/ama-t190-capability-composition.log`. The actual native Type supplies
list literals, default state, scalar clone, condition and index decoding; a
specialized algorithm returns the selected value 9 and the native consumer
observes `9:1:0`. Removing any of the five required operation bindings refuses
the real frontend without code generation. Managed invalid-literal execution
returns before the algorithm body. The initial isolated leaf compilation
reported an unused fixture diagnostic helper
(`/tmp/ama-t190-capability-initial.log`); the planned native-refusal case now uses
that helper to check an actual failure boundary. No production code changed.

Next step: concrete static-export native fields and independent typed headers.
All first-step runs are collected; no active jobs.

The first four-form native export composition reached the real frontend and
correctly rejected the test's mixed binding styles (named inputs with a
positional output): `/tmp/ama-t190-static-export-fields.log`. The fixture now
uses the named runtime result consistently. The new case checks four concrete
runtime input forms plus one output, zero native own formals, identical selected
resource identity and exact role ordinals. A concrete typed callback reads the
reference through its checked visitor without retaining it. Each native
interface is also written as the first include of a separate strict C probe;
probe text owns its include spelling beyond frontend teardown. The test removes
only its explicitly authored adapter companions before the next scenario.

Current run: `/tmp/ama-t190-static-export-fields-named.log`, session 64175 (now collected). Strict O2/Werror. Collect
before changing that leaf. Remaining work: open-dependency and lifecycle/layout
refusals, header/native consumers, sanitizers and final documentation acceptance.


The corrected four-form native interface and all its isolated headers passed
(`/tmp/ama-t190-static-export-fields-named.log`). Its checked reference callback
observes the original value 7, with a separate ordinary native consumer confirming
the returned value. Both runtime and static categories retain their own roles;
there are exactly four runtime inputs and one output and no native own formals.

Current run: session 77575, `/tmp/ama-t190-native-boundary-composition.log`,
strict O2/Werror. It adds four-form unresolved supplier refusals, actual incompatible
C entry declarations, previous/authored output preservation, all four native
construction/destruction status violations and real 64-byte aligned native
payloads across scalar/list operations. Collect before changing those inputs.
No production change or unused competing implementation has been introduced.


The first expanded run reached the subsequent publication assertion after the
invalid-C case (`/tmp/ama-t190-native-boundary-composition.log`). That case had
intentionally replaced an owned output with a marker and left the marker behind;
the publisher correctly refused this edited owned file. The test now compares
real previous output bytes directly without modifying them. This both strengthens
preservation evidence and removes cross-case output contamination; production
publication behavior is unchanged. The isolated rerun is
`/tmp/ama-t190-native-boundary-output-preserved.log` (collect launch session).

The first rerun retained the edited owned output from the intentionally failed
previous leaf, and correctly refused publication again
(`/tmp/ama-t190-native-boundary-output-preserved.log`). After collecting it,
`make -C tests/unit/codegen/native-templates clean` removed only that leaf's
preserved generated artifacts. Current clean strict run:
`/tmp/ama-t190-native-boundary-clean.log` (record launch session). No maintained
source or old failure receipt was removed.

Native admission and concrete scaffold contract consumers passed
(`/tmp/ama-t190-native-admission.log`, `/tmp/ama-t190-native-contracts.log`).
The clean expanded leaf exposed an incorrect lifecycle-case expectation:
`/tmp/ama-t190-native-boundary-clean.log`. Its selected resource destructor can run
while replacing initial state, before the algorithm enters; a malformed destructor must therefore abort before the consumer
prints, just like the other preparation failures. The expected output now reflects
that specified preparation boundary. No runtime implementation or error checks
were changed. The export fixture additionally routes its selected Type through
an actual named result alias and retains another public static output, keeping
four runtime inputs and one output unchanged. A prose writer failed after applying
these source edits; this append supplies the missing durable record.
Current strict run: session 80458,
`/tmp/ama-t190-native-boundary-lifecycle-boundary.log`. Collect before editing
that leaf; earlier launch sessions are collected. Documentation metadata session
43319 is also still to collect. No production change is introduced.


All eight expanded cases passed the corrected strict run
(`/tmp/ama-t190-native-boundary-lifecycle-boundary.log`), including actual
64-byte C alignment, four lifecycle transport refusals, four unresolved supplier
forms, independent headers and incompatible C entry rejection. Header probes
also now explicitly check that the native error naming macro is cleaned up,
alongside native/field/input/output macros. The complete metadata gate passed
2989 ready inputs with zero debt (`/tmp/ama-t190-docs-complete.log`). Current
step: strict final-input and sanitizer acceptance of the strengthened leaf;
no production change or obsolete alternate implementation is necessary.


Strict final macro-cleanup checks passed (`/tmp/ama-t190-final-strict.log`).
Initial actual Doxygen generation also passed
(`/tmp/ama-t190-doxygen-initial.log`); final archival prose will require a fresh
render. The invalid native entry case now additionally requires the real GCC
conflicting-types diagnostic, so an unrelated compilation failure cannot satisfy
its negative expectation. No executable implementation changed. Current next
run is the complete eight-case leaf under ASan/UBSan, including its compiled
child programs (`/tmp/ama-t190-sanitizers.log`, session 50967).


ASan/UBSan acceptance passed for all eight cases, including generated child
programs and the strengthened real GCC type-conflict cause assertion
(`/tmp/ama-t190-sanitizers.log`). Leak detection is disabled because LSan is not
available in this environment; counted frontend teardown is separate evidence.
A comment now describes pre-body initial-state replacement without claiming an
unmeasured first destructor site. Final strict run:
`/tmp/ama-t190-final-cause-assertion.log`, session 88202. No other
acceptance jobs remain active. After this run, archival and final Doxygen
fingerprint validation remain before the task commit.


## Acceptance

All runs are collected and passed, including final strict strengthened cause
assertions (`/tmp/ama-t190-final-cause-assertion.log`). No jobs remain active
before the final archival documentation generation. The eight cases and existing
native admission/scaffold contract consumers establish this boundary without a
production rewrite. There is no competing obsolete implementation to retain or
remove. Actual child programs cover native operations, checked borrows and real
C alignment; type checking alone is not claimed as execution evidence.

Reproduce from the repository root using
`C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`
with `make -C tests/unit/codegen/native-templates`,
`make -C tests/unit/compiler/native-admission` and
`make -C tests/unit/native/scaffold/contracts`.
The complete new leaf also passes with
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`,
`C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie'`
and `LINKER_FLAGS='-fsanitize=address,undefined -no-pie'`.

`make docs-complete` passed 2989 ready inputs with zero debt. Actual Doxygen
initially passed 2276 file pages, 8857 interfaces and 444/444 destinations, with
Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0. Regenerate after this full
archival prose with `python3 tools/documentation/build.py --doxygen doxygen`;
its final receipt is `/tmp/ama-t190-doxygen-archive.log`. Compare every rendered
input and staged source hash before the task commit. No specification or
conformance fingerprints changed. T196 still owns the independently established
stale conformance review gate; these checks are not a claim of a new green
full-language conformance run. [DONE](../../DONE.md#t190) preserves this task.
