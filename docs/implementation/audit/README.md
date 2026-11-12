# T001 implementation audit

This is an implementation decision/evidence log, not a handwritten file or API
reference. Generate that reference with `make docs` and Doxygen. The whole
[specification](../../../spec/README.md) remains authoritative.

## Snapshot and evidence boundaries

The audited implementation is commit `220b3ae`. No production code was changed
by this audit. [Snapshot metadata](snapshot.json) records the input revision,
inventory sizes and successful baseline test-log fingerprint. Later work must
refresh affected evidence instead of treating this snapshot as current forever.

[T132 removal closure](removal-closure.md) reconciles the later cleanup against
this immutable inventory and records its independent/composed evidence.

- [File inventory](files.csv): all 586 tracked paths under `src/`, `stdlib/`,
  `tools/`, `tests/`, `make/`, plus the root Makefile and Doxygen configuration.
  It includes code, ABI/module inputs, fixture expectations and checked-in data.
  Each path has a content fingerprint, decision, finding and follow-up tasks.
- [Requirement matrix](requirements.csv): one row for each of the 200 canonical
  rule definitions, with an implementation boundary, existing evidence and
  remaining task route. A partial baseline is not full current conformance.
- [Acceptance matrix](acceptance.csv): all 108 named rows of the canonical
  acceptance tables, preserved individually with current assessment and tasks.
- [Source probes](source-probes.json): eleven independent current-contract
  counterexamples, including exact input, invocation mode, expected result and
  observed diagnostic. They expose gaps; they are not passing acceptance tests.
- [Findings](findings.md): the reasons for removal/adaptation and the small
  refactoring/documentation tasks identified before continuing implementation.

The direct test-consumer column is measured, not inferred from filenames.
For each of the 57 actual test leaves, an additional temporary Make target used
GNU Make's `$(file ...)` to export its expanded `SOURCES`, `LOCAL_SOURCES` and
`COMPILE_COMMAND`. Each listed translation unit was then preprocessed with that
command, replacing dependency-writing flags by `-MM -MT audit -MF -`. Normalized
repository dependency paths give the recorded consumer relation, including
headers and included `common.c` fixtures. The query did not run the test target,
read generated C/object contents, or modify maintained inputs.
The query used the baseline root flags
`-O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L`, together with
each leaf's own definitions and `-UNDEBUG`, so optimization-dependent preprocessing
does not silently use the independent leaf's different default flags.

This relation establishes compilation/inclusion, not meaningful behavioral
coverage or independent testing of every child operation. Fifty-three C/header
paths have no direct test translation-unit consumer: these include native
companions compiled by emitted-project tests, `src/main.c` exercised through the
CLI, the Unicode generator, and assembly-only includes. They require explicit
functional evidence or added tests under T005/their owning tasks, not automatic
exemption. Every existing C/header lacks a Doxygen `@file` comment; T145–T151
must close that debt before T134 can complete.

## Commands actually run

`make test` completed with exit status zero. All 57 registered unit/integration
leaves ran through the real recursive parent tree, including generated-program,
native signature/inlining, ownership, continuation, rebuilding and harness
compositions. The log also includes two executions inside the harness's fixture
project; they are not counted as additional registered product suites.
The environment used GNU Make 4.4.1 and GCC 15.2.0 (Ubuntu 15.2.0-16ubuntu1).

`make docs-clean` followed by `make docs`, using Doxygen 1.11.0, generated the
entry page, file browser and tag entries for all 428 maintained C/header files.
Generation succeeded but emitted existing documentation/link warnings. It did
not establish current comment completeness or rendered specification-link
correctness; those are still T005/T134 work. Missing-Doxygen failure propagation
was checked in the preceding documentation-build commit.

For a source probe, write its `source` bytes to `<project>/main`, create
`<project>/value` containing `@!` plus LF, and run:

```sh
./ama --check <project> .main --root "$PWD/stdlib/modules"
```

Use `--status` instead of `--check` for the draft-TAB probe. Checking a draft as
an entry would fail for inactivity even with the missing TAB validation, so that
would not isolate the actual lexical gap. All eleven observed outcomes disagree
with the current specified outcomes. No generated C, binaries or object files
were inspected for this audit; normal compiler/test processes produced and ran
their own outputs.

The root Makefile's release optimization flags and the leaf's `-UNDEBUG` were
used in the successful baseline. ASan/UBSan, a second compiler, races and leak
certification were not rerun here; older reports remain historical for those
properties. T004/T120 must collect their required current evidence. A successful
old-subset test cannot certify new syntax, list membership, context isolation,
asynchrony or compiled-library embedding.

## Revalidated historical baseline

| Previously reported area | Current evidence and audit outcome |
|---|---|
| Components/build/tests | Recursive build and all registered leaves pass; compiler-dependent inclusion is measured. The old replacement-only component map is stale, not an instruction to discard the new tree. T002/T003/T004/T005 refresh contracts, independent boundaries and coverage. |
| Native ABI | Real C protocol, status, namespace, lifecycle, alignment and inlining compositions pass. The new integer size bridge, isolated contexts and compiled-library boundary are absent. |
| Sources/identities | UTF-8/XID, original spans, `name+/`, source diagnostics and collision tests pass for the old identity model. Multiword names, standard-root identity and draft lexical checks fail current probes; generator regeneration remains unverified. |
| Parser | Iterative structure, spans, ABI records, literals and failure cases pass the old tests. Current spacing, comments, inference-related structure, direct results and foreach are not accepted correctly. |
| Semantics/plans | Namespace, nominal transfers, separate graph analyses and resolved lifecycle/control plans have passing component/composition tests. New inferred/local scope, chain/service/control models are missing. |
| Check/native authoring | CLI checking and generated native header/skeleton publication tests pass for the old source subset. They do not validate current syntax, library exports or hand-authored C behavior generally. |
| Ownership | Stable pools, retirement, clone maps, staged replacement, overlap and tree protection pass. Keep these foundations; extend their context/membership/control boundaries. |
| Execution | Generated stackless projects execute and native entries are verified. Old generated globals/single-owner host policy prevent the required independent contexts; current result trees/service frames are absent. |
| Continuations | Explicit root/nested yields, resume, recursive clearing/preservation and new-caller tracing pass. Foreach cursor/origin copying and async enrollment/resource contracts are absent. |
| Lists/library | Scalar operations and standalone old list mutations pass. Generated whole-list actions explicitly fail; the old dispatcher and selector types are not a usable current list implementation. |
| Tools/release | Current export, relocation/editor/debugger, reactor/scheduler and parallel/channel acceptance does not exist. Their TODO tasks remain open. |

The audit found implementation and derived-documentation drift, not a reason to
change language requirements. No unresolved semantic decision is introduced by
the findings. Source acceptance that contradicts a rule must be removed or
replaced; reusable ownership, Unicode, diagnostics, ABI and build algorithms
remain candidates for verified adaptation rather than a blanket reset.
