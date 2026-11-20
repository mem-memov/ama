# Guarded source preparation and ordinary semantic composition

The production boundary implements
[COMP-02](../../spec/implementation/compiler.md#comp-02),
[TPL-06](../../spec/language/templates.md#tpl-06),
[COMP-04](../../spec/implementation/compiler.md#comp-04) and
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
[Specialized semantics](specialized-semantics.md) explains the closed owner graph;
[static relocation](static-relocation.md) records its source-snapshot consumer.

## One source model

A complete loaded Project supplies original AST/source identities. Static
inventory and native-definition admission precede guarded compilation roots.
Source lexical classification fixes ordinary declarations, inferred receivers,
foreach cells and every source use before solving their Type equations. Type
closure then supplies complete concrete owner keys and original source bindings.
Open definitions remain source definitions until a concrete application requires
them. Neither static producers nor unused open templates acquire runtime Types.

[Common semantic composition](../../src/compiler/resolve.c) consumes this accepted
closure and projects one ordinary Type per complete owner key. It runs native
protocol checks, initializer/body/control/transfer checking, eager and clone
relations, complete candidate certificates and shared constant startup/cleanup
analysis. Only successful outputs may be consumed. A failure retains its original
stage diagnostic and every borrowed source/equality/caller context until teardown.
There is no independent one-Type-per-source-module analyzer.

[Inference proof checking](../../src/semantic/inference/infer.c) requires the
accepted closure rather than replaying source events. It publishes the ordinary
checker prerequisite only after that proof exists. Actual source classification
and refinable Type equations belong to the static connection pipeline.

## Executable versus captured source consumers

Normal frontend completion builds executable storage/control plans and seals its
captured input graph. C signatures and native bodies are verified later through
the real C toolchain; semantic checking does not execute or parse opaque C bodies.

`compiler_frontend_check_snapshot` explicitly validates a caller-captured catalog
without an entry, executable plan or physical input-cache graph. Its caller owns
source/native versions. This permits future relocation paths without reading
nonexistent companion files. It uses the same admission, guarded Type closure and
ordinary semantic code as executable compilation. Source success is returned
explicitly; the composite `frontend.valid` certificate stays false without an
executable plan. Empty domains and open source
modules require no fabricated entry or default instance.

Both modes retain initialization credits before fallible work. Project/source
owners outlive inventory/closure, which outlive projected ordinary metadata and
plans. Reverse teardown covers every partially initialized prefix. No failed
mode publishes complete outputs or changes source documents.

## Evidence and independent stage consumers

`tests/unit/compiler/composition` compares actual frontend outputs with explicit
ordinary stages over accepted guarded closure, including native authoring,
source/error attribution, resolved plans and exhaustive allocation refusals.
Its comparator has its own maintained file rather than depending on a retired
concrete-only test leaf. `tests/unit/compiler/captured-catalog` checks future
native paths, closed exports and empty/open domains without cache authority.

Low-level semantic and planning fixtures use
[current source preparation](../../tests/unit/semantic/preparation.c), then invoke
the actual ordinary components they test. Valid source/storage/diagnostic cases
remain useful across this migration; comparisons of the removed raw pipeline and
its sole-purpose stage/allocation tests were deleted. The source-preparation
failure sweep injects every measured allocation before the real preparation, so
it cannot silently become vacuous when ordinary inference consumes a certificate.

The linked T193 record retains actual commands, failed boundary probes and current
acceptance status. Historical task counts are not a current whole-project result.
