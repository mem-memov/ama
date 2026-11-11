# Component contract template

Copy the relevant sections into a component design note and replace the prompts
with concrete statements. This template is guidance, not an accepted contract for
an unimplemented component. Small helpers may share a parent contract.

## Responsibility and requirements

Name the single transformation/service, the applicable requirement IDs, source
files, exposed declarations and consumers. Distinguish existing behavior from the
implementation target. Link requirements rather than copying language rules.

## Domain and representation

Specify mathematical input/output domains and their C representations. State valid
ranges, encoding, aliasing and lifetime requirements, initialization state, and
which inputs are rejected through normal errors. Name all environmental assumptions.

## Ownership and effects

Identify owners and borrowed views, allocations, mutation boundaries, retained
state, caller obligations and output ownership. Include partial-output effects and
external side effects. Do not promise rollback unless the specification requires it.

## Laws, preconditions and postconditions

Express the operation as a function or transition relation. State its success
predicate, invariants and postconditions and explain the permitted failure outcomes.
For each failure phase, identify unchanged state, consumed/finalized resources,
returned status/diagnostics and cleanup responsibility. Include arithmetic bounds.

## Algorithm and justification

Explain the general algorithm and why its branches cover the complete domain.
Give the inductive measure, algebraic derivation, invariant-preservation argument
or refinement relation. Relate the reasoning to actual source operations. Explain
why composing children preserves their ownership, values, spans and failure laws.

For an optimization, establish equivalence to the specified semantics over the
complete domain; agreement on examples alone is insufficient. Document assumptions
about native callbacks and the toolchain instead of treating them as proved code.

## Termination and complexity

State the decreasing measure, conditional termination where external operations
may block, and time/space bounds with named input parameters. Include owned memory,
auxiliary memory and relevant external-resource cost. Separate compiler-service
termination from arbitrary user-program nontermination.

## Tests and evidence

Map each law and boundary/failure partition to independently runnable tests and
real composition tests. State finite domains for exhaustive model checks. List the
commands actually run, results and material limits. Update tests and reasoning
together when the implementation changes; never invent evidence for future work.
