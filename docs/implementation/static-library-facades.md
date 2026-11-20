# Concrete template facades at the checked C boundary

## T194 contract and plan

The public C boundary selects concrete ordinary roots; static assemblies and open
constructors remain compile-time source. This task verifies composition of the
accepted template pipeline with the existing library model, packaging, public
transport and isolated native child ownership. No new export syntax or spec
change is authorized or needed.

Rules: [TPL-13](../../spec/language/templates.md#tpl-13),
[EMBED-01](../../spec/implementation/c-library.md#embed-01),
[EMBED-04](../../spec/implementation/c-library.md#embed-04), and
[EMBED-08](../../spec/implementation/c-library.md#embed-08).

1. Add exact negative model cases for open/static/generated selectors and missing
   dependencies, retaining supported native roots and transport refusal coverage.
2. Package concrete facades using named static exports and distinct closed
   substitutions. Compile independent C consumer translation units against the
   public header, run static/shared hosts and inspect identities/manifests.
3. Exercise concrete generic workers inside the independently compiled child
   used by the real native adapter supervision scenarios. Preserve the existing
   completion/yield/failure, peer isolation, teardown and code lifetime checks.
4. Run affected strict parents, focused ASan/UBSan, per-file coverage and actual
   Doxygen. Archive and commit only after required evidence passes.

Invariant: selecting the concrete facade fixes every required runtime Type before
transport/header generation. A static producer contributes equations and named
exports, never an instance or public C result slot. Independent host contexts
retain independent state even when specialization identities coincide. The
native boundary cannot turn a child yield into a retained C activation.

## Current step

T193 is committed as `b7e22fe`. Existing packaging already executes two separate
components through static, shared and unload/reload hosts. Existing supervision
executes a real separately compiled child and tests parent failure cleanup. Extend
these live consumers rather than creating a parallel library implementation.

## Steps 1–2: actual public consumers

The strict resolution suite passes with explicit refusals for open sources (even
when a closed specialization is already discovered), static roots including
closed graphs, fictitious generated/static selectors, unsupported static-input
records and missing ordinary closure. Existing unsupported owned/reference
transport and concrete native-profile checks remain in the same live suite.
Receipt: `/tmp/ama-t194-export-refusals.log`.

The packaging scenario now uses two ordinary facades with explicit named static
selections. The one source worker has two concrete captures, native text and
arbitrary-precision integer, with distinct generated full-key namespaces. Both
static exports are required and validated even when the facade uses only one;
the first probe incorrectly assumed only its selected export was materialized,
and was corrected to assert both exact capture Types and distinct identities.
No compiler rule or behavior changed. The static producer has no SemanticType;
each public facade still has exactly one runtime result and mapping.

Strict packaging passed: `/tmp/ama-t194-facade-package.log`. The same real
facades run in separate C translation units and a combined larger host through
static and shared artifacts, followed by explicit loader unload/reload. Direct
consumer translation units now include their self-contained public header first.
Existing allocation refusal, owned snapshots surviving context disposal,
detached native completion and code pin retirement remain exercised. Actual
manifest output retains the computed export identity.

## Step 3: template-backed native child

The separately compiled supervision child now copies its declared text request
through the concrete named static export of an ordinary generic worker before
its existing native calls and multiple yields. A new host case creates two
independent outer contexts: disposing one normally or after propagated child
failure must retire only its owned child, without executing another body. The
other context resumes twice and retains its own request/state. Existing probes
check parent/child busy gates and zero retained C helper frames.

Strict supervision passed: `/tmp/ama-t194-template-child-supervision.log`.
The new peer case covers both normal abandonment of a yielded owned child and
propagated failure. Exact created/disposed counts, native resource counts,
unchanged step/callback counts during disposal, peer resumption and eventual
code unloadability all pass.

The complete strict library parent passed:
`/tmp/ama-t194-library-parent.log`. All actual library model, transport,
execution, lifecycle, snapshot, packaging, native nesting, supervision and async
leaves passed. The complete per-file gate passed with **3012** inputs and zero
debt: `/tmp/ama-t194-metadata.log`.

The final strengthened integer facade compares the arbitrarily large copied
integer in both orders using the real native integer-before protocol; either
inequality changes its public text result to `wrong`. Thus the existing C host
checking `hello` observes the generic result, rather than merely compiling a
worker whose result is unused. This strengthening follows the parent package
run and is under the focused sanitizer run; rerun the strict package leaf too.

Active jobs: package ASan/UBSan session 37107,
`/tmp/ama-t194-package-sanitizers.log`; supervision ASan/UBSan session 92563,
`/tmp/ama-t194-supervision-sanitizers.log`. Next: collect both, final strict
package/metadata, archive, actual Doxygen and staged-input hash verification,
then commit T194. No production changes were required. LSan is unavailable;
counted owner/resource assertions provide separate teardown evidence.

## Observed-result fixture correction

The strengthened integer example initially attempted a literal assignment in a
branch. The source checker correctly rejected it: `Literal data requires a
declaration initializer or an entry input`. The strict diagnostic receipt is
`/tmp/ama-t194-package-final-strict.log`; the earlier sanitizer run stopped at
the same source refusal and is not sanitizer acceptance. The example now
initializes an ordinary typed `wrong` text variable and copies it in either
mismatch branch. No semantic relaxation or fallback is involved.

Supervision ASan/UBSan passed (`/tmp/ama-t194-supervision-sanitizers.log`),
including the new independent peer scenarios and existing child/parent
allocation-refusal sweeps. Current job: strict final observed-result package
session 6967, `/tmp/ama-t194-package-observed-strict.log`. After it passes, rerun
its sanitizer variant and final metadata against unchanged maintained inputs.

## Reading the facade

Informative example from the actual package fixture (`...integers` is its second
export):

```ama
# assembly: compile-time definitions, no runtime instance.
@...
...texts
.forward
 ...element < ..text
...integers
.forward
 ...element < ..integer
```

The ordinary concrete facade keeps its public `value` typed as native text; its
private worker uses one named export:

```ama
@>
value
..text
 |hello
@!
worker
.assembly
 ...texts
worker !
 < value
 > value
%
```

The explicit C configuration selects `.textfacade`, not `.assembly` or
`.forward`. Its result mapping is `output | run | value | text`. The two
compile-time exports add no two extra C return values. A host creates independent
contexts/instances and receives owned text snapshots; disposal cannot invalidate
a retained snapshot, and the component cannot unload while such owners or a
code pin remain. The integer facade independently exercises the other capture
and verifies a large integer exactly before yielding its ordinary text result.

Final strict observed-result packaging passes:
`/tmp/ama-t194-package-observed-strict.log`. The intermediate refused source
example above remains recorded as a fixture correction, never as acceptance.

## Collected execution acceptance

Final strict observed-result package passes:
`/tmp/ama-t194-package-observed-strict.log`. Final package ASan/UBSan passes:
`/tmp/ama-t194-package-observed-sanitizers.log`. Supervision ASan/UBSan and the
complete strict library parent pass as recorded above. Final unchanged-code
per-file validation passes with **3012** ready inputs and **zero** explicit debt:
`/tmp/ama-t194-final-metadata.log`. No test job remains active.

The result exercises ordinary concrete Ama facades, both native text/integer
captures, multiple static outputs, concrete native roots in existing nested
consumers, static/shared manifests/header consumers, independent snapshots,
foreign-owner refusal, child yielded/failed disposal, peer preservation and
detached native code lifetime. C receives no static binding or runtime Type
object. Exact closure and ordinary checking reject unsupported/open boundaries
before public publication. No compiler/library production modification, spec
change or conformance fingerprint update was needed. T196 still owns the known
independent conformance review gate.

Next acceptance step: actual Doxygen, archive the accepted task, regenerate for
archived inputs and compare rendered input hashes with staged bytes before the
T194 commit. Next ready implementation task is T195.

## T194 accepted result

Actual Doxygen passes (`/tmp/ama-t194-doxygen.log`): 2291 maintained
file pages, 8895 documented interfaces and 448/448 canonical destinations.
All required execution and per-file checks above are collected successes. The
complete accepted task moves to DONE in this task commit. Final archival inputs
must be regenerated and match staged bytes. Next ready task: T195.
