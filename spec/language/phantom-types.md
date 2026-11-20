# Phantom fields and nominal type markers

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a distinction without a stored value](#explanation-a-distinction-without-a-stored-value)
- [The standard phantom constructor](#the-standard-phantom-constructor)
- [Declaring a fixed or generic mark](#declaring-a-fixed-or-generic-mark)
- [Where the compiler obtains the mark](#where-the-compiler-obtains-the-mark)
- [What a phantom declaration contributes](#what-a-phantom-declaration-contributes)
- [Rejected uses](#rejected-uses)
- [Implementation consequences](#implementation-consequences)

<!-- /chapter-contents -->

## Explanation: a distinction without a stored value

A mass and a distance may use the same numerical representation while meaning
different things. An algorithm should not accidentally copy one into storage
reserved for the other merely because their bytes fit. A type marker gives the
compiler a name for that distinction; a phantom field makes the marker part of
an ordinary template's interface without creating a value to carry it.

The marker is a Type, not an instance. The phantom field is a declaration used
by the compiler, not a small or empty runtime object. The containing ordinary
module still has real instances, state and ordinary operations.

**Design rationale.** Writing the standard constructor in the type position
preserves the familiar name/type pair and makes the absence of runtime data
explicit at the declaration. Requiring a marker prevents an arbitrary
data-bearing Type from silently losing its storage. Keeping the marker in the
specialization identity gives libraries nominal distinctions without runtime
tags or changes to the representation of their actual data.

The admitted declaration expressions follow
[TPL-15](templates.md#tpl-15). Phantom declarations do not introduce an
independent static graph into an executable module.

## The standard phantom constructor

<a id="tpl-16"></a>

**TPL-16.** `..type.phantom` is a compiler-recognized standard-library static
type constructor with exactly one named Type formal, `...mark`. It does not
denote an ordinary executable module or a nonempty static producer with public
result fields. Its fully applied result is a closed **phantom declaration
Type** containing the supplied marker's exact nominal identity.

Recognize the constructor by its resolved complete standard-library logical
identity, including root kind and all components. A user module named
`.type.phantom`, a final component named `phantom`, or a similarly spelled
alias of another Type does not acquire this contract. Module resolution still
uses [SRC-01/SRC-02](source-and-names.md#src-01). This rule defines the
`..type.phantom` identity; translated standard-library names are a separate
design question.

The supplied argument must resolve to a closed nominal static type marker
under [TPL-14](static-results.md#tpl-14). Resolve aliases, static exports,
outer parameters and structural substitutions before testing this property.
The local spelling of a placeholder has no effect on the check. An ordinary
module Type, a phantom declaration Type, a residual Template or a nonempty
static producer is not a marker and is not an admissible final argument.

An open use can retain the `...mark` constraint while its enclosing template
is being described. For every concrete used specialization the argument must
be uniquely determined, closed and a marker before executable code is
generated. Omitting its argument is not a default marker. The usual
Type/Template distinction, specialization identity and exact-completion rules
apply to this constructor under
[TPL-05/TPL-07/TPL-08](templates.md#tpl-05).

## Declaring a fixed or generic mark

Use an unsuffixed declaration name and place the type expression at the same
indentation. Indent the named binding by one ASCII space. No initializer or
default follows a phantom declaration: it has no value to initialize.

**Example: fixed marker.** Assume `.weight` is a marker file containing only
`@...` and permitted layout. The following interface fragment fixes one mark:

```text
@<
input
..type.phantom
 ...mark < .weight
```

This fragment does not introduce a free `...unit` or any other template
parameter. The marker is already known. The `input` declaration does not ask
the caller to supply a marker value.

**Example: complete generic `.quantity`.** Assume `..integer` supplies the
ordinary numerical representation needed by the application:

```text
@<
input
..type.phantom
 ...mark < ...unit

value
..integer

@!
```

Here `...mark` names the constructor's formal. `...unit` names a Type parameter
of `.quantity`, introduced by its interface under
[TPL-02](templates.md#tpl-02). These are different scoped names. Writing
`...dimension` instead of `...unit` changes the outer parameter's name, not the
requirement that its final substitution be a marker. The left-hand name remains
`...mark` because it selects the constructor's defined formal.

Repeated occurrences of `...unit` in this interface denote one outer parameter.
An already scoped parameter may also be used in a body declaration; such a
declaration cannot introduce another free parameter. Shared declarations may
reuse interface parameters in ordinary type expressions, but cannot declare
phantom constants because constants require runtime values. Both `@<` and
`@>` may introduce parameters through the admitted type expressions under
TPL-02. A phantom declaration in `@>` is not a public static export: named
static results belong to [TPL-12](static-results.md#tpl-12).

**Example: two concrete specializations.** Assume `.weight` and `.length` are
distinct marker files:

```text
@!
mass
.quantity
 ...unit < .weight

distance
.quantity
 ...unit < .length
```

Each variable stores the ordinary state of its `.quantity` specialization.
Their Types are different because their marker bindings differ. No `.weight`
or `.length` instance is created. The `input` declaration takes no space inside
either instance; the ordinary `value` field retains its normal storage and
lifecycle behavior.

## Where the compiler obtains the mark

The ordinary declaration expression may explicitly bind a marker, as in the
preceding example. A permitted typed connection can also determine an omitted
marker parameter from another fully known specialization under
[TPL-03/TPL-04](templates.md#tpl-03). The compiler compares the complete
specialization structure, including phantom bindings, even when no runtime
field represents those bindings.

There is no ordinary runtime argument for a phantom field. Passing an integer
value to `value` does not determine `...unit`: that same representation can
belong to mass, distance or another marked quantity. Literal contents, a
variable's name, marker spelling, branch selection and the first executed call
do not supply missing type evidence. An unresolved or conflicting required
marker is a compilation error; no implicit generic marker or first-choice
fallback is allowed.

**Example: constraints, not value-based guessing.** In a caller, connecting a
field of known `.quantity(...unit = .weight)` Type to an otherwise open
`.quantity` field can determine the latter's unit through nominal Type
equality. That notation describes the resulting Type; it is not additional Ama
source syntax. Supplying only a `..integer` field instead supplies no such
equation about the unit and cannot choose `.weight`.

## What a phantom declaration contributes

A declaration whose expression establishes the phantom-constructor role is a
**phantom field**. Known aliases and selected exports preserve that role;
resolve the expression and marker constraints before admitting the concrete
specialization. A bare whole-field runtime Type parameter retains TPL-02's
runtime-Type constraint and cannot silently become phantom after substitution.
If a producer's result role is pending, obtain actual static evidence before
matching its runtime interface; argument counts never choose a phantom role or
invent its marker. A phantom declaration must have an unsuffixed name. Applying `&`, `[]` or `&[]` to
its declaration is invalid. Its name still participates in declaration scope
and duplicate-name checks; the name cannot be reused to create runtime storage
under another interpretation.

A phantom field has no runtime value, storage slot, address, object identity,
checked reference, payload, initialization, reset, clone, destruction, busy
state, saved position or method body. It is not an entry in the runtime input
or result sequence, whether written in `@<` or `@>`. It consumes no positional
argument/result position and has no omission, default, literal or transfer
action. A named ordinary binding cannot select it. Retained result reads and
direct result operations cannot select it either. Its name does not denote a
runtime source, receiver, condition, index or callable target. A shared
constant declaration whose resolved Type is a phantom declaration Type is
invalid: no phantom constant value exists.

The intrinsic phantom Type's own identity retains its exact marker. Preserve
every outer formal binding that determines a phantom mark in the enclosing
template's canonical substitution under [TPL-05](templates.md#tpl-05), even when
it has no stored representation. A fixed marker expression introduces no extra
outer formal or derived key input. Two instances of
the same complete marked specialization have independent ordinary state; two
specializations differing only in their marks are distinct nominal Types.
[TYPE-05](declarations.md#type-05) still rejects an implicit conversion between
them, even if their runtime layouts are identical. Wrapping the declaration
in an ordinary module or forwarding its Type through a static alias does not
erase either the marker check or the resulting nominal distinction.

An ordinary module that contains only phantom declarations is still an
ordinary module if its source is ordinary. Its real instances remain subject
to the normal identity, call and lifecycle rules for ordinary modules. Removing
its runtime storage or coalescing its identities requires the same observable
equivalence proof as any other optimization; the phantom-field rule alone does
not provide that proof.

## Rejected uses

**Examples: incomplete fragments with the stated errors.** The following
argument is an ordinary runtime Type, not a marker:

```text
input
..type.phantom
 ...mark < ..integer
```

The following declaration would require a runtime reference to a phantom
object that does not exist:

```text
input&
..type.phantom
 ...mark < .weight
```

The same rejection applies to `input[]` and `input&[]`. Declaring a runtime
field directly as `.weight` also remains invalid under TPL-14: the constructor
introduces a declaration role, not a way to instantiate a marker.

A phantom-field name cannot appear in an assignment, reference acquisition,
entry/resume call, retained read, ordinary input/result binding, literal
construction or collection operation. An alias whose final result is a
phantom declaration Type obeys the same restrictions as a direct constructor
application. A source spelling or an indirection cannot turn it into ordinary
storage.

## Implementation consequences

**Explanation.** The front end retains each phantom declaration's source name,
scope, resolved marker and constraints so diagnostics can explain an invalid
use. Specialization keys retain its complete binding. Layout and call lowering
then omit the declaration from per-instance storage and runtime interfaces;
no dummy object or runtime marker argument is needed. Type descriptors that
already identify distinct ordinary specializations may still represent their
nominal identities. They do not create marker instances or per-field payloads.

This is a semantic absence of runtime storage, not an optional optimization
that becomes available only after detecting an unused field. No additional
permission for runtime type generation, value-dependent type arguments, static
natural-number computation or type-recursion cycles follows from phantom
declarations. Existing composition, layout, specialization-creation, reference
and lifecycle checks continue to apply to the enclosing ordinary types.
