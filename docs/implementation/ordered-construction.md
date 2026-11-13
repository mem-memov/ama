# Ordered construction with final self identity

T049 integrates standalone current-instance sources with the real declaration
recipe and ownership lifecycle path under
[REF-01](../../spec/language/values-and-references.md#ref-01),
[RT-12](../../spec/implementation/runtime.md#rt-12),
[LIFE-01](../../spec/implementation/native-interface.md#life-01),
[LIFE-02](../../spec/implementation/native-interface.md#life-02),
[LIFE-03](../../spec/implementation/native-interface.md#life-03) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
The [stable pool](instance-ownership.md) reserves the final typed identity before
field initialization; the [lifecycle ledger](native-lifecycle.md) tracks the exact
completed prefix. This task adds a runtime source category for `&`, rather than
encoding self as an access to an invented field.

## Identity without reading or acquiring state

[Current-instance evaluation](../../src/runtime/program/values/self.c) checks the
source's nominal type, exact reserved/public handle and lifecycle, then returns a
scalar reference with no payload location, list buffer or acquisition. Trusted
private evaluation accepts Constructing or Live storage; normal evaluation still
requires a published identity. Fresh unused reservations and finalized state do
not form self values. Shared initialization has no current instance and refuses
this source. A running published current instance may form its identity without
becoming available for a second invocation.

Output is published only after validation. Lookup precedes every cell read, and
no payload field is inspected. The returned type/slot/generation is exactly the
reserved current identity. Registry lookup takes O(T) for T registered types;
slot lookup and source formation take O(1), with no allocation or scratch. Identity
formation does not weaken normal access or publication checks.

[Recipe resolution](../../src/runtime/program/construction/recipes.c) consumes the
reference through its existing typed source path. Declared reference fields copy
the handle; owned fields still require owned sources. The resolver callback is
given the current work's reserved handle, so child self is the child, not its
parent or dynamic caller. Earlier completed private fields remain available to
already checked declaration initializers. No module body runs during construction.

## Complete initialization and cleanup

Construction walks fields in declaration order, completing each owned child
before advancing the parent's initialized prefix. Each native payload selects
one default/literal/clone/size constructor after children are complete. Literal
construction directly selects its constructor, without default-then-overwrite.
All initializers and native resources live at their final private addresses;
publication changes visibility without relocating payloads.

Induction over field/child completion establishes that every credited child owns
one complete lifecycle and every uncredited child remains part of the registered
private work path. A constructor failure cleans its own partial native resources;
the runtime unwinds registered work and the credited ancestry. Successfully
constructed native payloads are destroyed before reverse-order children. Identity
is withdrawn before cleanup and generation advances before slot reuse. Thus a
self handle observed before failure cannot later identify another object, even
when allocation reuses its address. Sparse block reclamation permanently retires
its old range under the existing pool policy.

The existing iterative construction/cleanup bounds remain those of
[instance ownership](instance-ownership.md); adding current-instance formation
adds one O(T) lookup per self source and no ownership edge or recursive traversal.
A reference to self therefore introduces neither an ownership cycle nor a new
native cleanup obligation.

## Verification and remaining integration

Run `make -C tests/unit/runtime/program-construction`. The real program descriptors
combine one literal child, one default child, child self references, a parent self
reference and a reference to an earlier completed private child. The test checks
exact field-resolution order, constructor counts, final native addresses and
native-before-reverse-children cleanup. Public `ama_program_start` uses the same
canonical resolver and produces the same reference values.

Test-only observers record identities without hiding persistent Ama fields in
native payloads. During initialization, they verify that self forms privately
while external entry/resume/read and normal source evaluation still refuse.
Each child/root native constructor is failed in turn; every observed identity
must remain stale across eight retries, reduced generation exhaustion and
reclamation/regrowth. Every allocation in a successful construction is separately
failed, with unchanged output, complete private cleanup and a successful retry.
Running identity formation, wrong nominal type, shared initialization, stale
identity and unbegun reservation cases check the source boundary independently.
Counted resource ownership and address/undefined-behavior sanitizers verify cleanup.

T050/T051 retain clone-map and staged replacement integration, including final
replacement destinations. T053 and later emission tasks connect concrete planned
layouts to generated execution. The C-generation guard for standalone self stays
in place until those adapters can preserve the complete contract; this task does
not claim compiled Ama self execution or change the specification. The
[coverage registry](documentation/ready.json) maps each affected source/header and
test/build input to named cases and standalone header commands.
