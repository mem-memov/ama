# Planned storage and actual C layout composition

T053 verifies descriptor/emission/runtime handoffs under
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[RT-02](../../spec/implementation/runtime.md#rt-02),
[CABI-02](../../spec/implementation/native-interface.md#cabi-02),
[CABI-03](../../spec/implementation/native-interface.md#cabi-03),
[STACK-04](../../spec/implementation/compiler.md#stack-04),
[ARCH-05](../../spec/implementation/compiler.md#arch-05) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
It consumes the [validated typed plan](typed-plans.md),
[ownership services](instance-ownership.md) and
[shared storage](shared-storage.md). Existing emission already derives physical
layouts from the actual C compiler; this task supplies independent compiled
composition evidence rather than another layout calculator.

## One C representation, exact nominal descriptors

[Storage emission](../../src/codegen/emit/storage.c) declares each maintained
pool-backed type's scalar fields as `AmaHandle`, list fields as `AmaHandleList`,
then its actual native State or retained Ama placeholder. Field order/forms and
logical identities come from the validated plan. Native State definitions remain
handwritten C and may require extended alignment or padding.

[Field descriptors](../../src/codegen/emit/descriptors.c) use `offsetof` against
those same complete C structures. [Type setup](../../src/codegen/emit/setup.c)
uses `sizeof`, `_Alignof` and the actual native payload offset. Checked runtime
initialization validates extents, disjoint fields, power-of-two alignment and
representable slot arithmetic before constructing anything. The compiler does
not guess native layout from textual State members or cast unknown data through
a universal machine word.

Assembly-local nominal IDs identify exact resolved module objects, independently
of their native layout. Two different modules may have identical C State sizes
and alignments while retaining different IDs and generated view types. Runtime
binding checks the expected nominal ID and handle validity before applying its
C layout; equality of representations never grants a conversion.

Constants-only namespaces have no storage structure/object pool in the runtime
registry or context program table. Nominal IDs may have gaps: exact type lookup
does not require a placeholder for a namespace. Shared values use their actual
declared value types outside executable pools.

## Correctness and implementation bounds

For each generated type, one target C translation unit contains the complete
storage declaration, field descriptors, native adapters and runtime consumers.
C's `sizeof`/alignment/offset rules establish the physical representation; checked
descriptor initialization establishes that every field access falls within that
representation. Exact nominal lookup then selects that representation before
runtime binding. Induction over declared owned fields extends this argument to
composed views: each child has a checked descriptor of its own nominal type.
References/lists expose declared handle locations rather than hidden native edges.
Stable block reservation preserves actual aligned addresses after directory growth.

This remains the general materialized representation. No compact or elided live
instance layout is newly claimed by T053. Namespace pool omission is verified by
absence from the registry and absence of construction effects, not by demanding
an identical allocation-attempt trace. Future specialized layouts must repeat
observable transition/lifecycle comparisons and measure their actual target C
sizes under the optimization contract; a smaller C type alone does not prove
smaller aligned pool stride.

Emission scans type/field records and uses exact registered identities. Existing
linear model lookups add their documented type/member costs; no new production
algorithm or allocation policy is introduced. Probe compilation/native hook work
is verification work, not a compiler complexity claim.

## Executable probes

Run `make -C tests/unit/codegen/layouts`. The real source fixture has owned scalar,
reference, owning-list and reference-list fields; seventeen list elements force
materialized block growth. A literal and whole-value clone exercise construction,
a two-declaration shared dependency crosses a constants-only namespace, and an
actual native worker entry receives a typed owned parameter view.

Three native modules use an actual `_Alignas(64)` State with a seventeen-byte
marker, native resource, logical number and address-sensitive invariant. Helpers
have repeated local names qualified by generated module identities. The worker
checks its own and its parameter's real addresses/data during verified native
entry. Lifecycle hooks count defaults/literals/clones/destructors and reject
relocation or duplicate destruction.

A separately written C probe includes the published project and compiles against
its real declarations. It checks every descriptor kind, nominal ID and field
offset against C `offsetof`, concrete sizes/alignment, namespace exclusion and
all four field values. It creates/removes nineteen additional instances, verifying
old live addresses/data survive block growth. Shared payloads retain their own
alignment and values. Generated read/interface binders run under a valid explicit
native-borrow admission scope; an equal-layout foreign nominal handle refuses
before publishing a typed view. Actual native entry execution is checked separately
before that controlled boundary setup.

After host disposal, native constructor and destructor counts must balance for
every module. The probe reports measured layout: on the tested GCC target, State
is 64 bytes/aligned to 64, plain storage 64 bytes, worker storage 128 bytes and
root storage 192 bytes. These are measured evidence, not portable language sizes.
Four negative source cases reject same-representation module substitution for all
storage forms during semantics, before a plan is published.

`AMA_LAYOUT_SANITIZE=1` instruments the standalone compiled probe with address and
undefined-behavior sanitizers; the parent test can be instrumented independently.
The codegen parent supplies surrounding assembly/execution consumers. Header
coverage and reproducible Doxygen generation accompany the
[per-file registry](documentation/ready.json). [T061](generated-contexts.md) connects the separate context factory. The actual
layout probe now holds a context lease for all descriptor/storage observations
and releases it before disposing its host.
