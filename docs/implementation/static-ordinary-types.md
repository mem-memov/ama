# Ordinary annotation facts and independent static exports

[T217](../../DONE.md#t217) extends T212's ordinary-Type operation with exact
source-only annotation facts. Authority:
[TYPE-01](../../spec/language/declarations.md#type-01),
[TPL-04](../../spec/language/templates.md#tpl-04),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[CONST-03](../../spec/language/declarations.md#const-03) and
[CONST-06](../../spec/language/declarations.md#const-06) and
[TPL-15](../../spec/language/templates.md#tpl-15).
T212 and T217 acceptance commands, failures, corrections and limits are in DONE.
No specification change or executable admission is claimed by this component.

## Why an ordinary field differs from a static constructor value

An ordinary generic path creates an independent field inference map. Assume
`/example/demo/v1.0.0/src/container` has one ordinary input whose Type is `.element`:

```text
@<
number
//integer
text
//text
@!
first
/example/demo/v1.0.0/src/container
second
/example/demo/v1.0.0/src/container
first !
 < number
second !
 < text
```

**Informative typing example.** The intended field Types are container(integer)
and container(text), with independent runtime storage. Their omitted element
arguments are independent original field variables, later constrained by the
ordinary calls. T217 tests those equations through the existing Type operations;
T182 generates the ordinary call connections, and later tasks admit/emit code.

A static graph instead retains a residual constructor when it supplies no inputs:

```text
@...
.family
/example/demo/v1.0.0/src/container
```

Here `.family` is a Template, not a container instance Type. Equating a generic
ordinary field with that residual constructor is incorrect. The zero-binding
identity law is therefore suppressed for an ordinary FIELD-policy module path
whose known constructor retains formals. Its ordinary application operation
creates exact field omission variables. Static partial values, supplied bare
Type placeholders and nongeneric fixed Types preserve their identity law.
Namespace lookup still cannot infer missing constructor arguments.

## Source facts and actual uses

The publication arena owns `StaticOrdinaryType` records. Keys include exact
source environment, original root expression and optional actual context.
Normal records retain an immutable annotation source; actual records additionally
retain guarded result work. Export records retain the original producer/public
symbol, its independent root and publication. A selector never introduces a
using-module placeholder, and free input parameters are not public exports.

`static_ordinary_source` registers finite ordinary annotation metadata with a null
context. It can use a supplied nominal environment without inventing an actual
ordinary root. Generic source environments remain exact lexical identities,
not guesses about their future specialization. `static_ordinary_type` registers
a real use after validating its actual incoming context. Different use records
can share the same source/value term without sharing validation provenance or
runtime state. Copied syntax/environments/records do not count as membership.

`static_ordinary_equations` processes the original normal annotation and every
nested input operator. [T270](../../DONE.md#t270) retains a finite original-child
cache and uses the existing immutable operand/application kernels for each source.
The declaration root keeps FIELD omission; nested inputs keep PARTIAL omission.
Bare Type operands receive their own identity equation, so nested nominal
information can determine an owner parameter without executing that owner.
It can substitute a supplied Type, create source-owned field omission variables,
and add only the structural equations entailed by the annotation. It neither evaluates an ordinary
body/default/call nor reads a computed ordinary definition. Static graph modules
are outside this API's source domain. No removed generic interface-preview
operation or computed-interface exception is restored.

Metadata normal work creates no actual context, root, ready result use or
publication certificate. Its caller may query known Type facts for finite
interface matching. `static_ordinary_evaluate` still requires an actual use;
normal final resolution likewise requires its actual context and guarded source
publication. A known metadata value does not satisfy those proof requirements.
Unknown repeated ancestry remains Pending before another body is exposed;
unequal complete keys fail even after metadata has inferred the field Type.
Equal independently grounded keys can resume. Ordinary body validation remains
a separate registry obligation before executable admission.

READY from actual evaluation concerns structural work: bare unknown Type forwarding
can already be READY while the value remains open. Normal final resolution
separately requires a complete ordinary Type. Pending value queries preserve
their value output rather than initializing it; callers must initialize receivers.

## Independent supplier closure

Registration creates a consumer-local Type variable for a selected export, with
no equation to the supplying output. Expected Types, literals, calls and desired
members cannot fill supplier inputs. Source-only export equations may request
only the actual independent static compilation root. They cannot manufacture
the using ordinary module's root or an ordinary producer instance.

Close every supplier input, output and actual dependency before selecting the
original public name. The selected value must be a complete ordinary Type;
other outputs may be bound residual Templates. Only then add the consumer equality.
Incompatible existing consumer evidence fails instead of rebinding either side.
Repeated requests recheck equations and reuse original source/root publications.

Source-only exports can resolve independently because that proof is complete.
When an actual ordinary use is registered, it must retain its own validation edge
to the supplier even if metadata already knows the same value and root. That new
required edge is progress; duplicate edges or allocation snapshots are not. Normal
actual uses still enter their real guarded source paths. No origin search, default
instance, matching-signature guess or contextual factory completion is permitted.

## Ownership, exact progress and costs

Destroy publication records before their borrowed results, registry, guards,
applications, solver, terms and inventory. Terms and signatures retain only
longer-lived original source/environment identities. Equation failures are sticky,
retain original source provenance and leave public outputs untouched. Partial
records/facts remain owned until teardown; rollback is not promised.

Exact registration scans original source occurrences and retained records.
Field holes are keyed by original environment, declaration and foreign formal;
they are not a new free parameter or a source-visible variable. Omission remains
stable on retry. Applying a simple annotation processes a finite original operator,
not a recursive body or a search through candidate Types. Its costs are exact
membership/term/application kernel scans. Export lookup scans the finite output
list and uses guarded structural publication for closure and cycle handling.

The shared publication progress snapshot counts unique equations, metadata facts,
requirements, actual contexts, graph/cache records, completed local graphs and
retained registry dependencies. It excludes allocation/application/retry counts.
The finite original operators and guarded actual graph expansion bound this
monotone fact/obligation universe. A duplicate attempt cannot manufacture apparent
progress. This is separate from claiming every known metadata Type is validated.

## Acceptance boundary

`publication/ordinary` covers actual Type uses, separate generic field arguments,
independent exports and resource/identity boundaries. `publication/annotations`
covers source-only generic/supplied scopes, no invented caller root, separate
actual validation, pending/equal/unequal ancestry, new versus duplicate required
edges, consumer-independent exports and every measured preparation/equation
allocation refusal. Current graph/result/constant compositions remain required
regressions. File-specific/header evidence and final commands belong to the task
record and [ready.json](documentation/ready.json). The completed T217 record retains strict/sanitizer/build/Doxygen evidence; T182 consumes the new annotation facts
for ordinary typed operations, and later tasks perform executable admission.

## Complete normal operand facts

[T270](../../DONE.md#t270) tests nested nominal inference independently of actual
contexts and existing generic-call composition. The cache's collected flag is
only source traversal evidence; it never closes a Type, marks a body checked, or
supplies an incoming root. Preparation traverses the finite original tree with
explicit frames, then applies existing pure kernels in preorder. The solver
receives the same simultaneous equations independently of that schedule.
All cache/frame records remain publication-owned after refusal; their source
and operator pointers borrow longer-lived existing owners.

General static producer input/selected-value expressions retain T226 as their
implementation owner. T269's explicit refusal prevents the older zero-argument
ordinary-export path from discarding newly parsed inputs. The normal operand
pass does not bypass that boundary or invent supplier inputs from an expected
consumer Type. The [declaration-expression record](declaration-expressions.md)
contains the current task, proof, failure partitions and collected run evidence.
