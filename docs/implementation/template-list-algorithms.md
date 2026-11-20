# Generic algorithms over intrinsic lists

## T195 contract and steps

This task supplies executable Ama examples and composition tests for ordinary
module templates operating on the existing typed list primitive. It does not
replace its storage provider or add collection/native privileges.

Rules: [TPL-09](../../spec/language/templates.md#tpl-09),
[TPL-04](../../spec/language/templates.md#tpl-04),
[FLOW-05](../../spec/language/lists.md#flow-05), and
[LIST-10](../../spec/language/lists.md#list-10).

1. Implement readable reverse-copy, reference-reversal and mirror examples; a
   paired reverse composes ordinary algorithms. Keep type construction in
   separate static assemblies, including a partial constructor completed by a
   second producer and multiple named outputs.
2. Compile/run the maintained sources with integer and text elements, explicitly
   typed empty lists, independent same-key instances and owned/reference forms.
   Observe exact ordered output, original-list preservation and reference aliasing.
3. Exercise live insertion before the boundary and deletion of current elements,
   plus busy/refusal/copy boundaries and invalid applications. Reuse real existing
   list/native test infrastructure rather than another implementation.
4. Run strict composition/regression tests, sanitizers, per-file validation and
   Doxygen, archive evidence and commit.

## Algorithm invariants

Reverse-copy inserts the next input element at position zero of a separate
result. After k iterations its result is reverse(input[0:k]); input order and
ownership are unchanged. An owning result copies values, while a reference result
retains checked handles to the existing targets. Empty inputs require a concrete
element Type declaration, never a sample value.

Mirror copies input into its own result, then inserts each visited target at
position zero of that same result. The insertion is before the live boundary and
therefore not visited. After k original visits the list is reverse(input[0:k])
followed by the original input, and exactly n original entries are visited. Its
result is reverse(input) concatenated with input. No unconditional append loop
or assumption of a frozen iterator is used.

A separate draining reverse removes its current first entry after cloning it
into the result. Its private input copy loses one entry per iteration; corrected
boundary still reaches every surviving original entry. This demonstrates deletion
without invalidating or reusing the removed iteration reference.

Current step: establish maintained examples and the actual code-generation leaf.
T194 is committed as `26908bd`. No specification edits are authorized or needed.

## Initial executable findings

The maintained standalone example checks, compiles and executes silently:
`/tmp/ama-t195-example-check.log`, `/tmp/ama-t195-standalone-example.log`.
The actual test leaf copies exact named maintained sources into its isolated
fixture and observes Standard integer values via their declared index protocol
and UTF-8 through the native text view interface. It does not substitute source
text to construct Types.

Two fixture probes were correctly refused. Passing an iteration reference into
an owned observer parameter violates the storage form; the observer now receives
an explicitly materialized value (`% item&`), as required by ordinary copy rules.
Reading the drain algorithm input through a public-result selector was also
refused. The algorithm now declares an ordinary `remaining` integer result and
computes it through the actual list-length method after traversal. Neither
correction changes visibility or copying semantics. The prior receipts are
`/tmp/ama-t195-owned-algorithms.log` and `/tmp/ama-t195-algorithm-forms.log`.

Current strict job: session 26182, `/tmp/ama-t195-complete-algorithms.log`. It
includes owning reverse/mirror/current-element deletion, nested partial pair
composition, independent equal-key instances, empty owned/reference inputs,
aliases/repeated reference memberships, ordinary refusal and invalid category,
formal, storage and noncopyable-element applications. Next: collect it, add
the exact standalone source case to the maintained named test interface, register
per-file evidence and run final compositions/regressions and sanitizers.

## Collected algorithm forms and coverage inventory

The complete initial algorithm leaf passes:
`/tmp/ama-t195-complete-algorithms.log`. It observes exact order for integers and
text, live insertion/deletion, original-list preservation, equal-key independent
instances, repeated reference memberships, aliased target changes, empty inputs,
ordinary refusal, and invalid static/storage/capability applications.
The final interface additionally executes the exact maintained `main` bytes
through the same real frontend/codegen/runtime path.

The first per-file gate identified that its example inventory listed only two
old directory families. It therefore rejected the newly registered Ama examples
as absent inputs (`/tmp/ama-t195-metadata.log`). The repository convention is now
explicit and general: every maintained extensionless file under `examples/` is
Ama source, regardless of family. Prose and generated/build paths stay excluded.
The actual documentation scenario adds a differently named family and a
generated counterexample, rather than hard-coding this algorithm directory into
the tool. No compiler source lookup or language classification rule changes.

Active jobs: final strict algorithms session 32291
(`/tmp/ama-t195-final-strict.log`), list/specialization regressions session 78947
(`/tmp/ama-t195-list-regressions.log`), documentation inventory scenario session
4792 (`/tmp/ama-t195-documentation-inventory.log`), final complete metadata
session 68001 (`/tmp/ama-t195-final-metadata.log`). Next: collect all, run the
final algorithm sanitizer composition, then generate actual Doxygen and archive.

## Final strict acceptance

The final algorithm suite passes (`/tmp/ama-t195-final-strict.log`), including
the named exact standalone-source execution case. The strict specialization,
traversal, list-value, insertion, deletion and ordering regressions all pass:
`/tmp/ama-t195-list-regressions.log`. These retain busy grants/refusal, mutation,
copy/continuation, capability, missing-element and result-frame lifetime behavior
under the same compiler used by the examples. The documentation scenario passes
(`/tmp/ama-t195-documentation-inventory.log`); emitted compiler errors inside it
are deliberate rejected-header/contract negative controls, not failing runs.

Current sanitizer job: session 42043,
`/tmp/ama-t195-algorithm-sanitizers.log`. Complete per-file validation session
68001 is collected separately. The algorithms' exact invariant/domain and O(n²)
front-edit cost are documented in the example README. No template list-provider
or privileged source/native operation is introduced.

## T195 accepted result

Actual algorithm ASan/UBSan passes (`/tmp/ama-t195-algorithm-sanitizers.log`).
The final complete per-file gate passes with **3028** inputs and zero debt
(`/tmp/ama-t195-final-metadata.log`). Actual Doxygen passes
(`/tmp/ama-t195-doxygen.log`): 2297 maintained file pages, 8908 documented
interfaces and 450/450 canonical destinations. All required jobs are collected
successes. LSan is unavailable; counted owner teardown remains independent
evidence. T195 is archived with its current sources, tests, usage, invariants and
per-file registry. Regenerate for these archival inputs and verify staged hashes
before committing. No spec or conformance fingerprint edits are included.
T196 remains the next ready task and owns current conformance review.

The final parent-dispatch acceptance also passes
(`/tmp/ama-t195-parent-dispatch.log`): `make -C tests/unit/codegen
template-algorithms` builds and runs the registered child through the actual
parent Makefile. Per-file evidence explicitly registers this dispatch and the
changed coverage-inventory scenario, alongside the existing build/tool cases.
Repeat the final complete gate for these last registry associations and generate
Doxygen for this exact final note before comparing staged input hashes.
