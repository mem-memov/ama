# Deterministic products and exact changed-input checks

T118 checks [COMP-05](../../spec/implementation/compiler.md#comp-05) and
[DEP-03](../../spec/implementation/compiler.md#dep-03) through actual compiler
commands and verified executables. It complements the exact
[input-graph argument](project-inputs.md), [Make dependency checks](incremental-build.md)
and [owned-output publication](native-scaffolding.md).

## What is compared

`make -C tests/integration/products` runs three product C/assert cases and an
independent process-termination guard described in the
[resource acceptance review](resource-acceptance.md).
Each owns a project, copied real integer foundation and output directory under
Unicode paths containing spaces. The project calls through two Ama dependency
levels, including a physical `ветка+/лист` source, then invokes a typed native
observer. Inputs are complete byte inventories, including native C, ABI records
and a transitively included handwritten header.

The repeat case compares generated C, headers, tooling JSON and the ownership
manifest as opaque bytes after repeated emission and after deleting the entire
owned output directory and rebuilding at the same path. It also executes every
verified product and requires the expected output. Generated C is not interpreted
to infer behavior, and binaries/objects are not read for these comparisons.
Absolute source/output paths are fixed within this comparison: changing those
declared inputs need not produce identical tooling commands or source maps.

The changed-input case independently changes a compile definition, transitive
Ama literal, transitive native header, native C body and its paired ABI/helper
binding. Each next verified executable must expose the corresponding expected
value or output text. Every build preserves the exact current authored inventory.
A changed native header need not change a wrapper that includes it; observable
execution proves that the new dependency was compiled. The test therefore does
not require meaningless generated wrapper churn.

The boundary case adds an ordinary root that conflicts with an existing logical
module. Compilation must refuse and preserve all previous generated text; that
previous executable must still work. A controlled native technical failure then
reports the caller's actual Unicode source path and line. Moving that written
call by three blank lines and rebuilding must update the trace from line 7 to
line 10, with no old line remaining. Authored bytes again remain unchanged.

## Argument and complementary scales

The immutable input graph compares complete category/key/payload/edge sets and
computes the least fixed point of changed-consumer reachability. Its independent
three-vertex graph oracle and real closure captures cover transitive Ama/native
inputs and full root configuration. No timestamp or content-hash coincidence
stands in for source equality in that graph.

Generation is a function of the validated source/type/control/storage model and
explicit target/settings. Complete nominal identities and deterministic graph
orders make names and descriptors independent of incidental allocation history.
Changing the declared source location legitimately changes its source mapping.
The clean/reused byte comparison tests these properties through the real public
command, while independently expected runtime values prevent equal stale outputs
from masquerading as successful regeneration.

`make -C tests/integration/rebuild` independently exercises the real Make rules:
transitive-header consumers, added/removed deep sources, compiler/CPP/LTO settings,
link-only changes, parent test fixtures and overlapping configuration checks.
The harness counts actual compilation/link invocations and runs their products;
it does not use guessed sleeps as an invalidation oracle.

`make -C tests/unit/project/inputs` and
`make -C tests/unit/native/scaffold/ownership` cover exact dirty-graph laws and
refusal to overwrite unknown/edited generated files or handwritten companions.
`make -C tests/integration/library-build` additionally compares repeated public
library metadata, links a real SDK-free consumer and proves a failed C-tool
rebuild leaves the prior static/shared products usable. Identity/collision tests
remain in the source and library namespace components, rather than manufacturing
a cryptographic collision through filename spelling.

## Ownership and failure scope

All source mutations belong to a single test-owned case tree. Compilation uses
exact argument vectors; Unicode/spaces never pass through a shell. Successful
leaves clean normally and failures retain their fixtures. Test processes inspect
only their own outputs and never rewrite maintained fixtures or specifications.
Failed generation does not acquire ownership of conflicting source/output files.

These checks establish the current supported generated-source and build behavior.
They do not promise byte-identical arbitrary external linker output, relocate
absolute debug paths, authorize stale ABI artifacts, or replace T119/T120's
resource and supported-toolchain acceptance. The
[current conformance matrix](conformance/README.md) connects these concrete cases
to the existing reviewed requirements without changing language semantics.
