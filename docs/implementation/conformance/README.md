# Implementation evidence and specification alignment

The language is defined by [the specification](../../../spec/README.md). The
T196 review connects **214 requirements**, **150 acceptance rows** and **56
evidence groups** to the implementation, component arguments and named tests
accepted before the marker specification change. These counts describe that
reviewed edition. [The review](template-review.md) records the changed
obligations, delivered task evidence, exact algorithms and acceptance commands.

T220 adds [TPL-14](../../../spec/language/static-results.md#tpl-14) and its marker
acceptance scenario. T225 adds [declaration expressions](../../../spec/language/templates.md#tpl-15),
retained by [T227](../../../DONE.md#t227), which also states the
[generation/testing policy](../../../spec/introduction.md#core-06).
[T221](../../../DONE.md#t221) records accepted marker implementation and scoped
evidence. [T226](../../../TODO.md#t226) owns declaration-expression implementation,
and T237 owns whole-sprint evidence renewal. The existing historical matrix
retains its accepted fingerprints and does not acquire new claims from those
separate task results. Until alignment passes, `make conformance-check`
refuses this newer specification; updating hashes alone cannot establish support.
The earlier release results remain evidence for their recorded input revision.
T230 additionally specifies versioned packages, slash/relative paths, single-dot
static names, scoped path aliases and independent project tools. The
[project alignment queue](../../../TODO.md#planned-alignment-projects-paths-and-project-tools)
owns their future implementation. Neither the historical matrix nor its hashes
are updated by this specification task.
T229 adds CMake companions/build orchestration, exact launcher transports and
source inclusion under BUILD-01..05, CLI-01..04 and EMBED-09. The planned
T239–T242 tasks own their implementation and new consumer evidence; generation
examples and the unchanged earlier fingerprints do not establish that support.

Open executable templates and static modules are compilation inputs. Concrete
ordinary runtime Types pass guarded closure and ordinary admission before layout.
The list foundation remains the existing intrinsic implementation; template
algorithms are verified without claiming the deferred foundational rewrite.
Native definitions remain concrete leaves, and C exports use concrete facades.

This evidence mapping is separate from the complete clean release and final
documentation gates owned by T197/T198. Its checker compiles case interfaces;
actual behavior follows from the recorded executed component/composition runs.

[The machine-readable matrix](matrix.json) contains an individual explanation
for every obligation of that accepted edition. Each explanation names evidence
groups. A group names its
actual source boundary, local contract documents, independently runnable C cases
and distinct composition cases with their exact commands. Follow those links to
understand why a rule is implemented and what its tests observe. Shared evidence
is deliberate: several requirements can constrain one coherent operation.

The [T001 audit](../audit/README.md) remains an immutable record of the original
implementation. Its old statuses are not current omissions or alternative APIs.
The [documentation closure](../documentation/closure.md) records the preceding
per-file checkpoint; the live file registry continues to grow with new work.

## Run the check

```sh
make conformance-check
make -C tests/unit/conformance
```

After the pending alignment, the first command validates the current matrix
and compiles every selected named C test interface using that leaf's real Make
configuration. It does **not** run
all referenced tests or certify arbitrary C implementations by reading names.
Run their recorded commands to exercise their behavior. The second command runs
independent positive/negative fixtures and then checks this repository with the
same complete gate. The ordinary unit parent includes that leaf.

A successful evidence check means that the reviewed mapping is complete,
current and executable. Mathematical component arguments, assertion quality,
actual behavioral runs, supported target evidence and release checks remain
necessary. The [release report](release.md) links the broader product, resource,
toolchain and clean-example verification performed by the final roadmap gates.

## Exact authority, not inferred coverage

The checker implements a finite join of explicit records:

1. Reuse the canonical rule registry to obtain the exact requirement IDs and
   file/anchor destinations. Require a bijection with matrix requirement rows.
2. Parse the top-level canonical acceptance tables with pinned
   `markdown-it-py==3.0.0`. Require the exact ordered section, area and complete
   criteria digest for every row. Quoted examples, nested tables and fenced
   Markdown cannot invent obligations.
3. Require substantive individual arguments and valid evidence groups. Every
   group must be used, name real maintained sources and nonempty local contracts,
   and provide registered component and composition source/case/command triples.
   Those two witness sets must be distinct.
4. Compile the actual C case signatures. A case name mentioned in prose, a string
   or an unrelated source cannot satisfy the interface requirement.
5. Before and after those probes, compare the complete specification, live
   maintained-file inventory, input bytes, per-file case registry and argument
   bytes with the reviewed fingerprints.

All sets and traversals are finite. Exact key equality establishes requirement
coverage; ordered equality and criteria hashes establish acceptance identity.
The explicit group relation provides concrete witnesses, while compiling their
interfaces establishes their C existence/type. None of these steps infers
semantic correctness from an include graph, symbol prefix or test count.

Duplicate JSON keys, unexpected fields, incomplete inventories, duplicate rows,
unknown groups, stale files, changed arguments and open claims fail the complete
gate. Partial review may explicitly retain an `open` claim when invoking the
checker without `--complete`; it must explain the gap and never passes the
complete target. The current reviewed matrix has no open rows. Its explicit
list-provider/dependent-type scope boundaries remain documented in the canonical
specification and release guide; they are not silently counted as new features.

The scripts read maintained inputs and write only temporary compiler probes
under the ordinary build workspace. They do not execute Ama/native constructors,
change canonical specification bytes, regenerate claims or silently approve a
changed implementation. Documentation fixtures own their synthetic repositories
and never alter the project's specification to manufacture a passing result.

## Updating reviewed evidence

An authorized specification change, source/test change or changed argument makes
old evidence stale. Review the affected claims and their component/composition
cases, preserve existing unrelated evidence, then record the newly reviewed
fingerprints. Updating hashes without that review would defeat this contract.
There is deliberately no command that turns changed inputs into approved claims.
The matrix itself is review metadata rather than a maintained code input, so its
input fingerprint set has no self-referential hash.

T117 also tightens the existing documentation gate: every compiler Doxygen
contract block carries its own clickable canonical-rule context. A file-level
link does not stand in for a function's explanation. Existing helper explanations
remain intact; shared component obligations identify the canonical context where
that helper has no narrower standalone language rule. The documentation tests
check all compiler C/header/include files and reject links hidden in strings or
examples. The migration adds comments only, preserving executable bytes and
physical line counts. Opaque non-C fixture assets are fingerprinted as bytes,
without an accidental UTF-8 decoding requirement.

## Independent checker cases

`tests/unit/conformance/main.c` explicitly runs six C/assert cases:

- `it_rejects_duplicate_keys_invalid_schema_and_open_claims` checks malformed
  records and refusal to accept an incomplete review as complete.
- `it_matches_exact_canonical_rules_and_acceptance_rows` checks missing, changed,
  duplicate and misdirected obligations and evidence groups.
- `it_rejects_changed_input_registry_and_argument_authority` checks source,
  specification, registration and argument provenance changes.
- `it_checks_actual_component_and_composition_case_types` compiles real fixture
  C interfaces and rejects wrong types or reused witness sets.
- `it_parses_canonical_tables_without_inventing_nested_obligations` checks the
  actual Markdown parser, escaped criteria and nested/fenced false matrices.
- `it_checks_the_current_canonical_evidence_and_real_test_interfaces` checks all
  current records and actual selected repository C case interfaces.

The synthetic cases use a private Unicode/spaced Git root, a small canonical
fixture, two actual C cases and their real Make configuration. This is an
independent failure oracle for the checker, not a hand-edited copy of production
specification rules. Failure preserves the leaf's generated fixtures normally.
