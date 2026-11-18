# Static module templates

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: one definition, several concrete types](#explanation-one-definition-several-concrete-types)
- [Placeholder spelling](#placeholder-spelling)
- [Parameters and their scope](#parameters-and-their-scope)
- [Inferring types from typed connections](#inferring-types-from-typed-connections)
- [Nested template instances](#nested-template-instances)
- [Specialization identity and independent state](#specialization-identity-and-independent-state)
- [Checking a specialization before execution](#checking-a-specialization-before-execution)
- [Concrete native modules and template source distribution](#concrete-native-modules-and-template-source-distribution)
- [Type parameters and template parameters](#type-parameters-and-template-parameters)
- [Partial substitution and composition](#partial-substitution-and-composition)
- [Static result interfaces](#static-result-interfaces)
- [Templates and Ama's own lists](#templates-and-amas-own-lists)
- [Compiler obligations and design gates](#compiler-obligations-and-design-gates)
- [Finite composition and specialization creation](#finite-composition-and-specialization-creation)

<!-- /chapter-contents -->

## Explanation: one definition, several concrete types

A template is a module definition with named static parameters. Those parameters
stand for module types or type constructors. The compiler obtains concrete types
from typed values connected to the template, substitutes them into the program
structure, and checks the resulting module before generating executable code. Runtime values and
execution state remain ordinary Ama values and state.

For example, a queue can store different types of work without maintaining a
separate handwritten queue definition for each work type. A template may also
own a field of another template's type. That field has its own inferred static
bindings and its own runtime state.

**Design status.** The placeholder spelling, ordinary-call model, nested-field
model, constructor signatures and partial-application forms below record
the agreed design. The examples distinguish a template definition in a source
file, a constructor used during compilation, and a concrete runtime instance. The
[decision index](../../AMBIGUITIES.md#open-template-design-questions) records the
resolved questions for this sprint. The uniform naming profile, including
internal ASCII hyphens, follows [SRC-03](source-and-names.md#src-03). The
native/export boundary is settled by [TPL-13](#tpl-13); the list-foundation
rewrite is outside the current sprint
under [AMB-006](../../AMBIGUITIES.md#amb-006). Finite nested composition and the
prohibition on recursive specialization creation follow [TPL-11](#tpl-11).
Type evidence and result
Type introductions are settled below. Parameter-dependent shared constants and
explicit type namespaces follow [CONST-03/CONST-06](declarations.md#const-06).
Static Type/Template results, source-ordered positional interfaces and immutable
receiver aliases follow [TPL-12](static-results.md#tpl-12). An explicit static
output phase requires a completely determined producer while permitting its
returned constructor to retain its own residual parameters.
This chapter does not imply that the compiler implements templates. Do not
invent source forms or choose an interpretation for a newly discovered ambiguity.
The grammar recognizes only the source forms explicitly specified in
[the grammar](../grammar.md).

## Placeholder spelling

<a id="tpl-01"></a>

**TPL-01.** A placeholder type consists of exactly three adjoining ASCII dots
followed by one or more complete name components separated by single adjoining
dots. Each component follows [SRC-03](source-and-names.md#src-03), including its
Unicode, underscore, internal ASCII hyphen and single-space rules. There is no
terminating dot.

The whole spelling is one static name. In `...data.element type`, neither
`data` nor `element type` denotes a separately addressable parameter or field.
The components explain the parameter's purpose. They are not a physical path,
an ordinary/standard-library root selector, or an instruction to find a file.
No source with that name is required or loaded. A component may use any spelling
allowed by SRC-03 without an existing directory or module of that name.

Recognize `...` before the concrete-path prefixes `..` and `.`. The prefix
directly adjoins its first component; internal dots directly adjoin both
components. Empty components, bare `...` as a name, four leading dots, trailing
dots, internal doubled dots and spaces adjoining a dot are invalid. Bare `...`
is instead permitted solely as TPL-12's positional static-binding marker. Indentation and
the separator outside an operand remain outside the placeholder name.

A placeholder spelling is permitted only in the static positions defined here:
type expressions, static formal/signature declarations, static result names,
receiver aliases, static binding labels and the explicit constant namespace in
[CONST-03](declarations.md#const-03). It is not a value source, runtime variable,
entry/resume target, index, literal, or dotted member-selection syntax. In particular,
`...type.field` never means access to `field` through `...type`.

**Example: spellings, not complete modules.**

| Spelling | Interpretation |
|---|---|
| `...data.element type` | One parameter with two descriptive name components |
| `...data.key type` | A different parameter, despite the shared first component |
| `...имя типа.назначение` | The same naming rules with international words |
| `...data.key-2 type` | Internal ASCII hyphen followed by a digit; the space still joins two words of one component |
| `...data.key--type` | Invalid consecutive hyphens under SRC-03 |
| `...data.key -type` | Invalid leading hyphen in a later word under SRC-03 |
| `...data.key type-` | Invalid word-ending hyphen under SRC-03 |
| `...data .element` | Invalid space before a separator |
| `...data.element.` | Invalid terminating dot |

## Parameters and their scope

<a id="tpl-02"></a>

**TPL-02.** The interface sections `@<` and `@>` introduce a module template's
Type parameters. Only `@<` introduces Template parameters and their signatures.
Collect the whole interface before checking uses, according to these forms:

- A standalone `...name` declaration in `@<` introduces a **Type** parameter
  when it has no indented signature members. With a nonempty signature it
  introduces a **Template** parameter under TPL-07. Declare an explicit formal
  at most once; its later uses may refer to that same parameter.
- A bare placeholder used as a runtime input or result type introduces a **Type**
  parameter unless that name already denotes a collected static formal or
  static result, or a visible received alias. A known Template
  in that position is an application. A root input application follows TPL-07's
  complete-input-pattern rule when it has no static output phase; a runtime
  result application follows TPL-08's field rule. TPL-12 governs applications
  with an explicit static output phase.
- A placeholder used as a runtime input's type in `@<` with a nonempty named static
  input binding block introduces a **Template** parameter if it has no explicit static
  formal declaration and does not name a collected static result or visible
  received alias. Its expected
  residual signature is exactly those binding
  labels and their expression categories under TPL-07.
- In the right-hand static expressions of input or result type declarations,
  including a static result's defining expression under TPL-12, a previously
  undeclared bare placeholder introduces a local **Type** parameter.
  A Template used on
  that side must have its category/signature declared by one of the preceding
  Template introduction forms. Do not infer its category from an actual value.

For these introduction rules, a bare placeholder head has no static **input**
bindings. Static output actions do not change that eligibility, including an
output-only positional skip. A head in an eligible interface position can
therefore introduce Type while reading static results from that Type. It must
still satisfy the existing exclusions for collected formals, result names and
visible aliases. Output labels name the producer's static results; they never
contribute a Template parameter's expected input signature. An input binding,
including an omitted positional input, is not a bare-head occurrence.

**Example: complete interface with an output-only Type head.** Assume the
eventual concrete input type declares the static result `...element type`:

```text
@<
source&
...source type
 ...element type > ...received element type

@!
```

The interface head introduces the owner's Type parameter `...source type`.
The ordinary input supplies independent nominal type evidence; there is no
search for a producer by output name. The receiver introduces an immutable
alias under TPL-12, with metadata from the selected result, and does not occupy
a formal position. No Template signature is inferred from `...element type`.
An explicit standalone Type formal for `...source type` would make the head a
use of that formal instead, with the same category. Shared sections, body-only
expressions and constant namespaces still cannot introduce free parameters.

First collect explicit static result binders and explicit static formals.
Classify received aliases and genuine implicit introduction sites under
TPL-12's source-order rule; an alias's later interface uses cannot introduce
another owner parameter. Then collect Template-introducing patterns from `@<`
and their signatures, followed by remaining Type introductions from both interface sections,
excluding collected result names, foreign binding labels, output receiver
aliases and signature entries. A bare occurrence
does not independently redeclare an already known Template. A Type parameter
cannot be used as an applied constructor. Reject duplicate explicit binders or
formals. Any repeated Template patterns must
agree on the complete named, recursively kinded residual signature. Each
signature and parameter category is fixed for the definition, independently of
which specialization the compiler processes first.
An unknown applied placeholder first occurring in `@>` does not introduce a
Template: declare its signature through `@<` first. A static result may derive
a Template from a known constructor; its result binder is not a new Template
parameter. A result typed by a concrete template path may introduce
local Type parameters on the right of its named bindings without introducing a
local Template head.
Signatures must be finite. Cyclic inferred signature dependencies that require
an infinite category/signature tree are static errors before specialization;
the compiler must detect the cycle rather than expand it indefinitely.

The left-hand labels of static bindings and the entries inside a static formal's
signature belong to the selected foreign constructor interface. They introduce
no parameters in the surrounding module. A bare signature entry declares a Type
slot of that interface; an entry with its own nested signature declares a Template slot.
For example, a signature containing `...value type` does not make that name
available as a Type parameter in the surrounding body.

Repeated local introductions of the same complete name refer to one parameter;
different complete names introduce independent parameters. Parameter order for
canonical identity is the order of first local introductions in the interface,
first `@<`, then `@>`, visiting an explicit formal at its declaration and
traversing each declaration's type/defining expression before the following
declaration. Foreign labels, result binders, receiver aliases and signature
entries do not occupy positions in this order. TPL-12 uses this original order,
restricted to actual residual formals, for positional static inputs; static
outputs have their separate textual `@>` order.

The parameter identity comprises its defining module identity and complete
placeholder name. Identical spelling in another template introduces a different
parameter. Physical submodule nesting does not inherit parameters under
[SRC-05](source-and-names.md#src-05). Ordinary variables and type parameters have
different roles: the variable holds runtime state; its placeholder describes
the module type of that state.

Static expressions in the Ama body may reuse local parameters and static results
introduced through that template's interface, and visible received aliases
under TPL-12. Foreign static binding labels there
still designate the selected constructor's own formals. A new local placeholder
appearing only as an unbound source in the body is a static error, including in an unreachable
branch or empty-loop body. Explicit static Template-formal declarations are permitted
only in `@<`; bare Type-formal declarations are likewise input-only. Receiving
a static alias under TPL-12 does not introduce a free parameter. The body cannot
add implicit type parameters to its enclosing module. Shared
`@=` declarations do not introduce type parameters. They may reuse parameters
collected from the whole interface under
[CONST-06](declarations.md#const-06), even though the shared section is written
before that interface. A constant-read namespace likewise reuses scoped
parameters and never introduces one.

The ordinary declaration's storage suffix remains independent of its module
type. The same parameter can type an owned scalar, a scalar reference, an owning
list and a reference list. Substitution never changes these four storage forms.
For a reference the parameter denotes the target module type; for a list it
denotes the element module type, not a new implicit list-of-list type.

**Example: an input-section fragment.**

```text
@<
key
...data.key type

value
...data.value type

other value
...data.value type
```

`value` and `other value` require the same module type. `key` is independent and
may have either that same type or a different one. Reusing `...data.value type`
in a result or body type line is valid. Introducing `...data.result type` in
`@>` is also valid; introducing it only in the body is invalid.

**Example: complete `.default result`.** Assume the eventual concrete type
supports ordinary default construction:

```text
@>
result
...result type

@!
```

There is no input section. The result declaration introduces a Type parameter
of `.default result` itself. An explicitly typed receiving variable, a static
binding or another permitted typed connection can determine it under TPL-03.
Different producer declarations may therefore have different specializations.
Every invocation of one declared producer still has one fixed result type.

## Inferring types from typed connections

<a id="tpl-03"></a>

**TPL-03.** An ordinary entry call keeps the existing named/positional argument
syntax. The caller passes values, references or lists under
[CALL-01/CALL-02](calls-and-results.md#call-01). Generate scoped type-equality
constraints from every statically checked connection below. Information may
propagate in either direction; the written direction of runtime value transfer
does not limit static type inference.

| Connection | Required type constraint |
|---|---|
| Ordinary input binding | Actual source module type equals the selected input's declared type expression |
| Entry, resume or retained-result transfer | Selected result type expression equals the receiver's module type |
| Declaration initializer or input/result default with a typed source | Declared type expression equals that source's type |
| Owned assignment, including a receiver-inferred field | Source type equals receiver type |
| Reference formation, copying or rebinding | Declared target types agree; a self-reference contributes its enclosing specialization's nominal type |
| Whole-list transfer or element transfer | Corresponding element module types agree, with the written scalar/list operation retained |

Each source/receiver keeps its ordinary lexical environment and lookup rules,
including selected result operations under CALL-09. Typed constants contribute
their declared type under the ordinary constant rules, including dependent
declarations in [CONST-06](declarations.md#const-06). The namespace of a constant
read must first resolve under [CONST-03](declarations.md#const-03); requested
member names and receiver types cannot choose an otherwise open namespace.
Reference and list types contribute their statically known
target/element type even without live values. Ownership, reference/value and
scalar/list compatibility are checked separately under
[TYPE-05](declarations.md#type-05); an equality never inserts a conversion,
reference acquisition, dereference or storage suffix.

A known symbolic type contributes structure and equations, not necessarily a
concrete answer. For example, connecting two independently open fields equates
their type variables; a later concrete source or typed receiver may determine
both. Ordinary aliases and references to the same declared field constrain the
same specialization variables, not new variables per alias or call.

Repeated uses of one placeholder require one identical nominal type, including
any specialization bindings. Distinct placeholders are independent until an
explicit program connection imposes equality. There is no implicit conversion,
common-supertype choice, filesystem search, name matching between templates,
or runtime dispatch on the supplied value's contents.

The compiler must satisfy all applicable static constraints together. It must
not select the first executed call, prefer an earlier branch, ignore a conflicting
call in a mutually exclusive branch, or make specialization depend on reachability
at runtime. Input/output binding modes, omission, default evaluation, busy checks
and result transfer retain their ordinary rules after specialization.
All declared typed defaults constrain their fields even when a call supplies an
argument. A supplied input still suppresses the default's runtime construction
under CALL-08. Static checking does not execute a default or a module body.

**Example: a typed default.** If an input typed by `...value type` has a default
copied from an existing text constant, that source constrains the parameter to
text. Supplying an integer at a call conflicts with that constraint even though
the default would not execute there. A literal default `|0` alone does not impose
integer: it is checked against the type determined by other constraints.

Known types are evidence even when a list is empty or a reference has no live
target. Their contents/liveness still follow the ordinary runtime rules. Literal
bytes such as `|0` have no intrinsic module type and cannot select one by their
spelling. A discarded result, an untyped receiver introduced by TYPE-06, or an
equation between unknown types alone supplies no concrete type identity. Literal,
copy, condition, index and callable capabilities are checked after the relevant
type is known; they do not select a type by searching for a supporting module.

An explicitly typed output receiver can determine a producer's result-only Type
parameter or parameters of a returned template specialization. A first-occurring
receiver introduced by TYPE-06 instead gets a type equation from that selected
result. It supplies no independent concrete type merely by being introduced.
Later permitted typed connections may solve that equation.

Receiver declaration identities and name availability remain source ordered
under [TYPE-06](declarations.md#type-06). A symbolic type may be refined after
processing later constructs; this does not make a later inferred declaration
visible at an earlier use, change a receiver's declaration site, permit a
self-dependent new receiver, or remove storage suffix checks. Type constraints
do not reorder value initialization: every initializer source must still be
initialized when its value is read, and persistent fields are still constructed
before the body under TYPE-02/TYPE-04.

Required concrete bindings must ultimately be unique under TPL-10. Missing
evidence cannot be repaired with a default type, implicit instance, runtime
reachability choice, body inspection of a native implementation, or search for
an unknown callee whose result would match. If a target's nominal type/interface
is not known yet, retain its constraint until another typed connection determines
it; report underdetermination if no such evidence exists.

A named constructor pattern on an input additionally uses the exact nominal
decomposition defined by TPL-07. The complete input type, including every actual
slot binding, remains part of the constraint; extracting a family does not erase
the old key/value types from the runtime interface. Explicit static formals
allocate no ordinary input positions, and named static bindings do not consume
runtime argument positions or change an ordinary call's binding mode.

### Examples: expected result types and later information

**Example: caller fragment using `.default result` above.**

```text
integer result
..integer

text result
..text

integer producer
.default result

text producer
.default result

integer producer !
 > integer result

text producer !
 > text result
```

The predeclared receiving types determine two owner specializations: one with
`...result type` equal to integer and one equal to text. Sending both outputs of
the same producer declaration to those incompatible receivers is a type error,
including when its calls appear in mutually exclusive branches.

**Example: a caller fragment using TPL-04's `.forward`.**

```text
known
..integer
 |7

worker
.forward

worker !
 < |0
 > inferred

inferred
 known
```

The first `> inferred` introduces a persistent field whose type is initially
equated to `.forward`'s result parameter. The later assignment equates it to the
known integer type. That fixes the worker specialization and permits checking
the literal `|0` against the integer constructor. It does not execute the later
assignment early. The worker and inferred field have their final concrete types
and ordinary default construction before the body executes.

With the later assignment removed and no other type evidence, this example is
underdetermined: `|0` and the new receiver do not choose a type. An explicitly
typed receiver would supply the same integer constraint directly at the call.

**Example: a result field whose type specializes another template.** Assume
`.box` declares one Type formal `...element type` and supports the construction
and copying used here. The following is a complete `.box producer` definition:

```text
@>
result
.box
 ...element type < ...produced element type

@!
```

The new local Type parameter is introduced on the binding's right-hand side in
`@>`. A receiver whose type is a box of integers fixes that parameter to integer;
a separate producer field may specialize for a box of text. Each result is an
ordinary owned box with a closed concrete type. A bare `.box` result without that
named local parameter instead uses only field inference variables under TPL-08:
it does not implicitly make its owner generic, and all instances of one concrete
owner must agree on that field's inferred box type.

## Nested template instances

<a id="tpl-04"></a>

**TPL-04.** A template may explicitly declare fields whose type path names
another template, or whose type expression applies a Template parameter under
TPL-08. No special nested-call syntax is required. The nested
template introduces its parameters in its own interface; those declarations do not
introduce names into the enclosing template's scope. Explicitly passing an
outer value to a nested input relates their module types under TPL-03, even
when their placeholder names differ.

Each field declaration has one fixed specialization within each concrete
specialization of its declaring module. The inference identity is the declaring
owner specialization plus the field declaration, not a runtime object address,
call site, alias, or textual access path. Different field declarations can have
different bindings. All runtime instances of one concrete owner type use the
same specialized type for the same field declaration. Do not silently add a
parameter to an ordinary owner to accommodate incompatible field uses.

An outer template's field can remain symbolically dependent on an outer
parameter while the outer definition is open. Before a concrete owner instance
is constructed, every required nested type must be concrete. TYPE-02/STATE-01
still construct persistent fields at owner creation, including fields written
inside branches and loops. The first runtime call does not allocate a hidden
specialization or change the type of existing state.

**Example: complete module `.forward`.** Assume each chosen type supports the
ordinary default construction and owned copying used here.

```text
@<
value
...inner.value type

@>
result
...inner.value type

@!
result
 value
```

**Example: complete module `.processor`.**

```text
@<
input
...outer.data type

@>
output
...outer.data type

@!
step
.forward

step !
 < input
 > output
```

The `< input` connection requires the inner value type to equal the outer data
type. An integer specialization of `.processor` contains an integer
specialization of `.forward`; a text specialization contains a text one.
The original placeholder names remain local and do not have to match.

**Example: complete caller module.** The standard types are assumed to support
the literals and copying shown; `.processor` is the preceding definition.

```text
@!
number
..integer
 |42

text
..text
 |Ama

number processor
.processor

text processor
.processor

number processor !
 < number
 > number result

text processor !
 < text
 > text result
```

The two explicitly declared processors receive independent specializations.
Their `step` fields specialize in turn. TYPE-06 introduces the two result fields
from the already determined selected result types. No placeholder occurs in
this caller and no type arguments are written at either call.

**Example: complete module with two independently specialized fields.**

```text
@<
first
...data.first type

second
...data.second type

@!
first step
.forward

second step
.forward

first step !
 < first

second step !
 < second
```

Passing an integer and text makes the two fields different specializations.
Calling `first step` with both inputs instead requires equal types and rejects
that integer/text application. If two instances have the same concrete owner
type, their corresponding `first step` fields cannot independently choose
different types. For a third nesting level the same explicit-connection rule
applies again; there is no implicit inheritance down the ownership tree.

## Specialization identity and independent state

<a id="tpl-05"></a>

**TPL-05.** A concrete specialization's nominal identity comprises the original
template definition's full logical module identity and the complete normalized
binding of its static parameters. Preserve parameter identities, parameter kinds
and all bindings, including a binding unused by the generated body. Concrete
type bindings use their full nominal identities, including root kind and any
own specialization bindings. Native C names and physical generated filenames
are representations of this identity, not its definition.

Equivalent substitutions reached through different nesting or partial-binding
routes have one specialization identity. Normalize by the original parameter
identities in their declared order; application order, aliases, path basenames
and coincidentally identical layouts cannot change or merge that identity.
Static inputs normalize to original formal identities, whether their source
form is named or positional under TPL-12. Derived static results and receiver
aliases are not additional producer-key components. Returning a Type preserves
that Type's own original identity rather than creating a producer-qualified
wrapper. Permuting distinct named argument bindings
to an already defined constructor in the same supplying environment preserves
their meaning and canonical result. Reordering introductions in the definition
itself may change its declared parameter order under TPL-02; that is not a
permutation of arguments to an unchanged definition. Moving a nested expression
that introduces a received alias across that alias's use is not a permutation
in the same lexical environment. A Template-valued binding includes
its constructor origin, fixed bindings and residual signature under TPL-08;
it cannot be identified by its signature or remaining arity alone.
Distinct template definitions remain nominally distinct. NAME-01/NAME-02's exact
encoding and collision checks apply to the extended identities.

One specialization identity shares type information and generated code. It does
not share mutable instances, fields, busy flags, results or continuations.
Two declared owned fields with the same specialization still own independent
state; an explicit reference may alias an existing instance under ordinary rules.
Shared `@=` values belong to each concrete specialization once per execution
context under STATE-03, not once per object or once across all specializations.

Reference target compatibility and list element compatibility include the whole
specialization identity. A list remains homogeneous. A reference cannot alternate
between incompatible specializations merely because their original template
definition is the same.

## Checking a specialization before execution

<a id="tpl-06"></a>

**TPL-06.** Before emitting executable code or constructing runtime state, all
used specializations and their required dependencies must be fully concrete and
pass ordinary Ama checking. An open template/partial binding is compile-time
information, not an executable value or an incompletely initialized instance.
An ordinary variable or reference carries one concrete nominal type at runtime.

Check syntax, placeholder scope and resolvable names at definition time. After
substitution, check storage compatibility, every body operation, interface arity,
defaults, literal/copy/condition/index capabilities, lifecycle and ownership
graphs, and constant initialization. A provided argument does not excuse a
statically invalid declared default. This does not execute an unused default:
CALL-08 still suppresses its runtime construction when an input is supplied.
Runtime branch outcomes do not disable static checks;
there is no silent removal of invalid operations or automatic selection of an
alternative specialization.

Templates do not add universal comparison, arithmetic, implicit conversions,
structural subtyping or access to another module's private fields. An ordinary
module instance passed by reference may supply a predicate/comparison operation;
its concrete callable interface must match the template's actual calls after
substitution. Type checking alone does not prove ordering, hashing or other
algebraic laws required by a library algorithm.

A known concrete native module may be a type argument, with its ordinary ABI and
capabilities. [TPL-13](#tpl-13) excludes native templates and defines source
distribution of open Ama templates. Export through concrete ordinary modules
follows [EMBED-01](../implementation/c-library.md#embed-01).
Specialization-qualified constant reads follow
[CONST-03/CONST-06](declarations.md#const-06) independently of that native boundary.

A general conversion may introduce independent input and result Type parameters
through the corresponding interface declarations, and may receive constructors
through TPL-07's static signature mechanism. A body-inferred receiver does not
introduce another user-visible parameter of its owner. Empty generic containers
likewise need determining type constraints; missing runtime elements provide no
type evidence. Every runtime result has one fixed type per concrete owner
specialization. Static result definitions and concrete producer closure follow
TPL-12; a returned Template may retain its own residual signature.

## Concrete native modules and template source distribution

**Explanation.** An Ama template organizes operations over supplied types. A
native module provides a concrete implementation boundary with a checked C
interface. Using a native integer type in an Ama template therefore does not
require writing a generic C implementation. The compiler specializes the Ama
definition and uses that integer module's existing concrete native contract.

**Design rationale.** Keeping native modules concrete avoids a second template
system for authored C representations, generated headers and lifecycle hooks.
Native modules are leaves at the implementation boundary. This description does
not make their declared Ama state a leaf in the ownership graph: concrete fields,
checked references and their existing lifecycle rules still apply.

<a id="tpl-13"></a>

**TPL-13.** A module whose implementation section is `@C` must have no own static
Type or Template parameters. Determine that parameter set by the complete
interface collection and scope classification in TPL-02/TPL-12, before accepting
the native definition or generating its authoring header. This restriction applies
to explicit formals, implicit Type introductions through runtime input/result
types or interface expressions, and implicit Template introductions through
input patterns. An unused formal is still a formal. Supplying concrete arguments
at every application does not make an otherwise generic `@C` definition valid.
Report the native implementation marker and the parameter introduction site;
do not synthesize a default specialization, erase the type through `void *`,
or generate a specialization-dependent C companion as a fallback.

A concrete native module remains an ordinary Type argument for an Ama-defined
template. Its nominal identity, copy/default/condition/index capabilities,
representation and lifecycle follow their existing contracts; substitution adds
no operation that it does not provide. Its native authoring interface remains
the concrete interface specified by
[CABI-01/CABI-06](../implementation/native-interface.md#cabi-01).

The restriction concerns the native module's own parameters, not every occurrence
of `...`. A foreign static binding label is not a local formal. A known closed
template application may supply a native module's declared field or interface
Type, subject to ordinary ownership, capability and native-access checks. Bound
static result names and received aliases remain governed by TPL-12; they do not
independently make a module a template. A returned constructor's own locally bound
residual parameters are distinct from free producer parameters. Static results
never become native runtime input/output positions. Every type used for native
runtime storage must be uniquely concrete before its header/layout is generated,
under TPL-06 and CABI-06. An unresolved field binding is not permission to create
a native template.

**Example: TPL-04's complete `.forward`, repeated for this boundary.** Supplied
types must support ordinary default construction and the owned copy used here:

```text
@<
value
...inner.value type

@>
result
...inner.value type

@!
result
 value
```

**Example: caller body fragment.** Assume the ordinary concrete native module
`..integer` supports its declared literal and copy operations:

```text
number
..integer
 |42

worker
.forward

worker !
 < number
 > copied
```

The call determines `.forward`'s element Type as `..integer`. The generated
specialization has an Ama body; `..integer` acquires no template parameters or
new native implementation. Replacing `.forward`'s `@!` section with `@C` would
be rejected even if this were its only caller, because its interface introduces
`...inner.value type`.

### Source delivery and the concrete C boundary

Distribute an open Ama template as its Ama source definition. A consuming project
loads that definition, resolves its dependencies, applies its own static
bindings and checks every required concrete specialization before generating
executable code. Partial constructors remain compile-time information under
TPL-07/TPL-08/TPL-12; they are not objects exported through a C runtime interface.
No serialized open-template ABI, binary-only template interface or runtime
specialization mechanism is introduced by this source-delivery contract.

A source distribution must supply the required Ama definitions, or make them
available through the consuming project's explicitly configured dependencies.
Required concrete native modules retain their normal Ama interfaces, authored
companions and external C dependencies; the consumer checks them under the
existing native build contract. Source delivery does not require embedding the
implementation of an independently supplied external C library in Ama files.

Apply the existing exact source-name and root-resolution rules in
[SRC-01/SRC-02](source-and-names.md#src-01) and
[DEP-01 through DEP-03](../implementation/compiler.md#dep-01). Packaging does not
create another namespace, implicit import, type-erasure scheme, or version-based
identity rule. The same original logical template identity and normalized
bindings retain one specialization identity under TPL-05. Conflicting definitions
of one logical module, missing dependencies, invalid bindings and unresolved
required types are diagnosed through the ordinary compiler rules.

For use by a C application, select a concrete ordinary module as an exported
root under [EMBED-01](../implementation/c-library.md#embed-01). A non-template Ama
facade may contain the required closed template specialization and expose a
concrete runtime interface. This preserves native-module identity and avoids
passing an open constructor through C. Direct selection of a template
specialization in export configuration is outside the current sprint; this
contract introduces no configuration syntax for static export arguments.

## Type parameters and template parameters

<a id="tpl-07"></a>

**TPL-07.** Distinguish two static parameter categories:

| Category | Meaning of its binding |
|---|---|
| Type | One module type, concrete or symbolically dependent on parameters in an enclosing composition |
| Template | A type constructor with a nonempty expected named residual signature, possibly carrying prior partial bindings |

A constructor is compile-time information describing how static arguments form
a type. It has no runtime state or entry point. A concrete specialization of a
template is a Type, even though the compiler retains its constructor origin and
complete argument metadata. Applying a constructor to all its residual formals
produces a Type; leaving some formals open produces a Template. A Type and a
Template are different categories, even if they have similar names or one is the
origin of the other. Category checking never performs a runtime call.

**Explanation: types are not numeric values.** These are the two static parameter
categories. A number stored in `..integer`, including a list position or length,
is an ordinary runtime value, not another static argument category. Different
integer contents do not produce different nominal types or specialization keys.
Reading a shared constant does not evaluate its contents as a type expression.
Composition may build different concrete types from different named Type and
Template arguments; it does not introduce value-indexed types, type-level
integer arithmetic or proofs that a mutable list has a particular length.

### Named signatures and explicit declarations

A signature is a finite, nonempty set of distinct complete formal names with
their categories. A Template slot recursively includes its own expected
signature. Names match exactly under TPL-01; order in a signature is not a
parameter position. Two signatures match if their name sets are equal and each
corresponding category and recursively nested signature matches. Extra residual
formals, missing formals, renamed formals and category differences are errors.
Prior bindings are not residual formals and are not listed in that signature.

An explicit static formal occupies its own declaration in `@<`. A bare name
with no indented signature declares Type; a name with a nonempty signature
declares Template. The distinction is structural and applies recursively:

```text
...element type

...dictionary family
 ...key type
 ...value type
```

There is no trailing `<` on either header. A signature entry is indented exactly
one level deeper. A bare entry declares a Type slot; an entry with its own
deeper nonempty signature declares a Template slot. Comments and blank lines
are layout, not signature entries. No runtime name, storage suffix, default, literal or ordinary
initializer belongs to this declaration. It allocates no runtime field or
ordinary input position. Its actual Type or constructor is supplied by a static
binding on the containing module's type expression, or constrained by an input
pattern referring to this formal. Missing information is not filled with a
default Type or constructor. Explicit static results in `@>` have a required
second expression line under TPL-12; they do not use this input-formal form.

**Example: a recursive signature fragment in `@<`.**

```text
...algorithm family
 ...container family
  ...element type
 ...payload type
```

The algorithm's `...container family` formal expects a constructor with exactly
one remaining Type slot named `...element type`. The algorithm's other formal,
`...payload type`, expects a Type. These signature entries describe the foreign
algorithm interface; they introduce no surrounding local parameters named
`...container family`, `...element type` or `...payload type`.

### Where a template is defined and where it is used

**Walkthrough: definition and explicit application.** Assume a hypothetical
module `.dictionary` has the following input-section fragment; its implementation
and results are outside this fragment:

```text
@<
key&
...key type

value&
...value type
```

The source file `.dictionary` defines this constructor. Its original static
formal names are exactly `...key type` and `...value type`, both Type. The input
variables `key&` and `value&` are runtime references with independent storage
roles. Their placeholder types do not themselves name references.

A complete module `.factory` can receive that constructor as a static argument:

```text
@<
...dictionary family
 ...key type
 ...value type

@!
first
...dictionary family
 ...key type < ..text
 ...value type < ..integer

second
...dictionary family
 ...value type < ..integer
 ...key type < ..text
```

Assume `.dictionary` supports ordinary default construction of these fields.
The two declarations apply the received constructor to the same named types in
different source orders. They have the same concrete type and independent state.
The following declaration fragment selects the actual constructor for `.factory`:

```text
builder
.factory
 ...dictionary family < .dictionary
```

`.factory` is a source module; `...dictionary family` is its local Template
parameter; `.dictionary` is the source constructor supplied to that parameter.
The generated concrete dictionary type is a virtual module in compiler memory,
not a file created alongside `.dictionary`. The field `builder` is a runtime
instance of a concrete `.factory` specialization; its static argument creates
no additional runtime argument. No `!` is needed to apply a type constructor.

### Input patterns that recover a constructor

A runtime input may declare a constructor-application pattern. Its root is a
Template parameter or a concrete module path naming a template; its children
are named static bindings. A concrete path fixes the nominal origin and
introduces no local Template parameter. If a placeholder root has
no explicit static formal declaration, the children introduce its expected
residual signature: their labels are its formal names, and their right-hand
expression categories determine the formal categories. Bare new right-hand
placeholders introduce Type under TPL-02; a Template on that side needs its own
declared signature. The category is never inferred by trying the first actual.

Without a static output phase, at the root of a runtime input type every residual formal of the constructor
must be bound explicitly exactly once, including when its head is a concrete
template path. An explicit prior signature must agree with that complete binding
set. An omitted residual at this root is an error:
name its local binding in the pattern instead. This makes the input type a
function of declared static parameters and prevents hidden owner parameters.
For example, an input typed `.dictionary` alone is incomplete for the two-slot
definition above. Binding both `...key type` and `...value type` to local Type
parameters is a valid fixed-origin input pattern. A bare local Type placeholder
instead accepts the complete concrete dictionary type as one Type without
decomposing it or introducing a Template.

The coverage rule concerns a root input type without an output phase; a nested
Template-valued argument may remain partial, and shared/body/runtime-result
declarations may omit residual bindings under TPL-08. A static output phase
instead invokes TPL-12's uniform producer-inference and closure rule. Implicit
Template introduction still requires named input-pattern labels; positional
bindings alone require an already known constructor/signature.

Match such a pattern only against a type with known nominal specialization
metadata. For a pattern with expected residual signature R matched to a closed
actual of original constructor D:

1. R must name actual original formals of D with exactly compatible categories
   and recursive signatures.
2. Recover the root constructor as D with precisely the R formals open. Keep
   every other original binding fixed, including unused bindings. Never reopen
   an unlisted slot or discard another slot's binding.
3. Constrain each actual R-slot binding to its explicit right-hand expression.
   These constraints determine local Type/Template parameters through the same
   scoped structure as other static constraints.
4. Retain the complete input type as the application of that constructor to
   those expressions. All its argument types remain in the owner's identity
   and interface; the constructor alone does not describe the input's layout.

Every occurrence of the same Template parameter must recover the same origin,
fixed bindings and residual signature. Different families with equal layouts
or signatures are not interchangeable actuals for that one parameter. A module
with no such nominal template metadata cannot satisfy a constructor-discovery
pattern. There is no lookup by filename resemblance, common body or structure.
Nested constructor-valued expressions use TPL-10's exact structural rule.

The same declared constructor application may also be constrained by a typed
result receiver, initializer, assignment or another TPL-03 connection. Its
signature still determines exactly which original slots are open. If the root
is a Template parameter, its category/signature must already have been introduced
through `@<`; inferring its binding does not introduce a new Template in `@>` or
the body. TPL-10 applies the same nominal recovery to these equations. A bare
Type variable instead receives the complete nominal Type without decomposition.

**Example: input-section fragment of `.remap`.**

```text
@<
source&
...dictionary family
 ...key type < ...key type
 ...value type < ...previous value type

replacement&
...replacement type
```

Passing a dictionary with text keys and integer values establishes the original
dictionary constructor as `...dictionary family`, `..text` as the local
`...key type`, and `..integer` as the local `...previous value type`. Passing a
text `replacement&` additionally establishes `...replacement type` as `..text`.
The complete source type still includes its integer value binding. A different
source value type gives a different `.remap` specialization, even when its body
does not read the old values. Any additional dictionary formals not listed here
remain fixed in the captured family and participate in its identity.

In `...key type < ...key type`, the left name is a formal of the selected
dictionary; the right name is local to `.remap`. The explicit binding connects
them. Renaming only the right name to `...source key type` and updating its local
uses preserves the connection. Renaming the left name to
`...dictionary.key type` is an unknown-formal error for the `.dictionary`
definition above: that whole spelling is a different name, not qualification.

### Category and interface errors

Unknown formal names, duplicate bindings, incompatible categories and unequal
residual signatures are static errors. TPL-12 defines positional static inputs,
outputs and skips separately from ordinary runtime `<`/`>` bindings. There is
no automatic formal renaming, guessed adapter or implicit opening of a concrete
Type passed where a Template is expected. Constructor
recovery requires a declared constructor application with an exact signature.
Ordinary calls keep their existing named/positional value bindings under TPL-03.

## Partial substitution and composition

<a id="tpl-08"></a>

**TPL-08.** Static substitution operates on bound program/type structure, never
on source text. It replaces references to specific formal parameter identities
in their lexical environment. It preserves caller/callee scope and cannot capture
an unrelated parameter or variable with the same spelling.

A static expression starts with a concrete module path or a visible static name.
A path resolves to its exact original module definition; a local placeholder
resolves in the supplying template's scope. An unapplied ordinary module denotes
a Type; an unapplied template path denotes its constructor. A Type parameter
denotes a Type, and a Template parameter denotes its bound constructor. Static
results and receiver aliases denote their defined Type or constructor under
TPL-12. Input application never changes a Type into a Template.

A named static binding has the form `...formal < expression`, with exactly one
separator on each side of `<`. Its line is indented one level below the expression
it applies to. The right-hand expression may have its own deeper binding block.
All bindings at one level apply to that level's constructor; deeper bindings
apply to their immediate right-hand expression, not to a sibling or ancestor.
The complete left-hand name selects an original residual formal of that
constructor. The right-hand side resolves in the supplying lexical environment.
Names used as foreign labels never enter that environment. TPL-12 normalizes
positional static inputs to the same original formal identities and defines the
separate static output phase. An expression with any static output binding
requests a concrete producer, with scoped inference terms for omitted inputs;
the omission rules below apply when no such phase is present.

On a runtime declaration, this block follows the type line and precedes all
ordinary runtime initializer sources. Static bindings and initializers are each
one level below the declaration's name/type lines, but their recognized forms
are different. A static binding after the first runtime initializer is invalid.
Blank lines and correctly indented comments may separate entries; they do not
change their owner or indentation. An empty binding block adds no syntax.

**Example: declaration fragment with both kinds of input.** Assume `.forward`
is the TPL-04 template and `source` is an initialized owned instance of its
specialization with `...inner.value type` bound to `..integer`:

```text
worker
.forward
 ...inner.value type < ..integer
 source
```

The static line determines `worker`'s concrete type. The final source copies
runtime contents into that declared instance under the ordinary initializer
rules; it must have the complete specialized `.forward` type. An integer value
itself would be incompatible with that destination. Static arguments do not
initialize the `value` input field with runtime contents.

A partial substitution binds a subset of the constructor's residual formals.
Each supplied expression must have the formal's declared category and, for
Template, exact expected residual signature. Unknown or repeated names fail.
A previously fixed formal is no longer residual; rebinding it is an error even
if the repeated expression would be equal. Applying input bindings to a Type is an
error. Distinct named bindings at one level are simultaneous in their supplying
lexical environment; a binding's foreign label cannot define another binding's
source. Nested static output receivers have their explicit lexical visibility
under TPL-12; moving a declaration across a use can change that environment.

Normalize the result as the original constructor identity, its map of fixed
original formals to normalized static expressions, and the remaining original
formal signature. Recursively normalize supplied constructors and captured
dependencies. Store/compare formals in original declaration order for canonical
identity. TPL-12's positional normalization uses that same original order,
restricted to actual residuals. Repeated aliases to the same
local parameter preserve its identity. No binding is dropped because generated
code does not use it. If residual formals remain, the result is Template; if none
remain, it is Type, possibly still symbolically dependent on surrounding
parameters before the enclosing specialization closes.

An outer template may receive a constructor, supply its own parameters to that
constructor and use the resulting type/template. Named bindings refer to the
selected constructor's own formal identities; the supplied expressions refer to
the explicit supplying environment. Identical names never create connections
by themselves. Nested ordinary calls in TPL-04 already establish symbolic type
equalities without additional call syntax; independently passing a residual
constructor additionally requires TPL-07's signature contract.

### Partial arguments and inferred field types

In a static argument or static result expression without an output phase,
omitted residuals stay bound formals of the
resulting constructor. They are available for later application or forwarding;
the compiler does not fill them with fresh runtime types just because the
expression occurs within a field declaration. A fully applied Type cannot
satisfy a Template formal, and an open Template cannot satisfy a Type formal.

At the root of a shared-constant, body-field or runtime-result declaration's type
expression without an output phase, omitted residuals instead receive fresh inference variables of their
declared categories and signatures. Identify each variable by its concrete owner specialization, lexical
field declaration and residual formal identity. Different fields get independent
variables; repeated calls through the same field constrain the same variables.
No new user-visible placeholder or implicit owner parameter is introduced.
Known static bindings and TPL-03's typed connections can determine these variables.
All required bindings must be uniquely concrete before construction/codegen.
This is an inference operation of the compiler, not partial runtime storage or
late selection by the first executed call. A named Type parameter introduced in
`@>` is a parameter of its owner; an omitted residual here remains only a field
inference variable and cannot implicitly parameterize that owner. Root input
patterns without an output phase use TPL-07's explicit-coverage rule instead. A shared constant's inferred
type still obeys [CONST-06](declarations.md#const-06); no runtime input supplies
its value. Constant-read namespace expressions are static selections, not field
declarations, and without an output phase do not receive these generated
inference variables. TPL-12 explicitly supplies producer inference terms when
any expression contains static output bindings, including a namespace or
otherwise partial argument expression. A returned constructor's own residual
signature remains distinct from those producer-input inference terms.

**Example: complete `.typed consumer`.** Assume the supplied family has the
dictionary interface from TPL-07, with a fixed key type and only value type open:

```text
@<
...received family
 ...value type

sample&
...element type

@!
storage
...received family

storage !
 value& < sample&
```

The static formal does not consume an ordinary input position; `sample&` does.
The `storage` declaration applies its family's remaining value formal to a fresh
field inference variable. The ordinary call constrains that variable to
`...element type`. This body is checked against the actual family's complete
interface; the signature alone does not promise an input called `value&`.

Declaring two such fields and calling one with an integer and the other with text
can give different concrete types. Calling the same field with both requires
those types to be equal and fails for integer/text. All objects of one concrete
owner type retain the same type for each corresponding field under TPL-04.

### Forwarding a partial constructor without a runtime sample

**Example: complete `.remap` using `.typed consumer` above.** The source input
provides the constructor and its previous bindings. `replacement&` provides the
desired new value type. The example describes types and construction; actual
dictionary enumeration/copying would require the ordinary dictionary methods.

```text
@<
source&
...dictionary family
 ...key type < ...key type
 ...value type < ...previous value type

replacement&
...replacement type

@!
step
.typed consumer
 ...received family < ...dictionary family
  ...key type < ...key type

step !
 sample& < replacement&
```

The outer `.remap` recovers the dictionary family from the nominal source type.
Inside the static argument to `.typed consumer`, it binds that family's key
formal to the outer key Type. Its value formal stays open, exactly matching
`...received family`'s one-slot signature. The ordinary `step !` input determines
the consumer's element Type, and its `storage !` input in turn determines the
dictionary field's value Type. This is three-level structural composition.
The original source retains its previous value type. No dictionary sample,
temporary instance or runtime call is needed to transmit the partial constructor.

All owned fields are still created under TYPE-04 before their bodies execute.
Static application itself executes no module body, default, initializer, copy or
native effect. A new dictionary has ordinary fresh contents; copying source keys
or assigning replacement values requires explicit runtime operations.

**Example: equivalence of staged and direct bindings.** If a consumer supplies
`...value type < ..integer` when applying the text-key partial family, the final
type is the same specialization as this direct declaration:

```text
dictionary
.dictionary
 ...key type < ..text
 ...value type < ..integer
```

Changing the order of these two lines does not change that type. Creating two
objects of it still creates two independent states. Partially binding the value
instead of the key produces a different residual signature and is not an
acceptable argument to a consumer requiring exactly `...value type` open.

Residual formals and captured dependencies are different. A dictionary whose
key is bound to an outer parameter still depends on that parameter even after
its value formal is bound. Conversely, a template-valued argument may retain
its own declared formals without making the receiving outer specialization open:
those formals are locally bound constructor parameters, not unresolved free
variables. Every type actually used as executable storage must nevertheless be
concrete under TPL-06.

Omission of a runtime `<` binding, an empty positional `<`, `>` or a literal
retains its existing call/default semantics. It does not perform static partial
application. Static maps are attached to declaration type expressions or explicit
constant namespaces under [CONST-03](declarations.md#const-03), never to
an ordinary `!`/`%` operation's argument sequence. An open constructor cannot be
an Ama runtime result, mutable field, checked reference or dynamically replaceable
type. It may be passed or returned through the static Template forms defined
here and in TPL-12.

### Invalid applications

**Examples: rejection cases, with the preceding interfaces assumed.**

| Case | Required result |
|---|---|
| Bind `...dictionary.key type` on `.dictionary` | Unknown formal: it is a different complete name from `...key type` |
| Bind `...key type` twice at one level | Duplicate static binding, even if both expressions agree |
| Supply a fully concrete dictionary Type for a Template formal | Wrong category; no implicit reopening |
| Supply open `.dictionary` for a Type formal | Wrong category; explicitly bind all residuals first |
| Pass the two-slot dictionary family to the one-slot `.typed consumer` | Residual-signature mismatch |
| Bind the already fixed key of a text-key family | Nonresidual-formal error |
| Leave a root input-pattern slot unlisted | Incomplete input-pattern signature; name its binding explicitly |
| Use a signature leaf as a surrounding local Type without introducing it | Out-of-scope placeholder |
| Introduce a new right-hand placeholder only in the body | Invalid local introduction under TPL-02 |
| Put a static binding after an ordinary initializer | Invalid ordering |
| Omit an operand of a named static binding | Invalid static syntax; only positional `... <`/`... >` admit skips |
| Infer signatures where F contains G's signature and G contains F's | Infinite signature dependency; static error before expansion |
| Leave a field's residual type with no determining constraints | Underdetermined runtime type; no default type is selected |

## Static result interfaces

[TPL-12](static-results.md#tpl-12) defines two-line static results, their
source-ordered output interface, named/positional projections and immutable
receiver aliases. A completely determined producer can return either a concrete
Type or a Template with its own residual signature. Static outputs add exact
constraints without executing bodies or creating runtime producer instances.
The chapter also states how output definitions interact with nominal identity,
source visibility, constant namespaces and specialization-creation provenance.

## Templates and Ama's own lists

<a id="tpl-09"></a>

**TPL-09.** Templates may implement reusable algorithms and containers over the
existing list primitives. They may also factor the typed implementation of the
language's own lists, provided every existing collection, lifecycle, identity,
continuation, failure and complexity contract remains satisfied. A wrapper over
a built-in list is not evidence that the list's primitive foundation is itself
implemented in Ama.

A language-list declaration supplies its element type statically even when the
list is empty and no method ever runs. An implementation must not require a
runtime sample element, first insertion or hidden default instance to discover
that type. TPL-08 now provides named static application of an ordinary template's
Type formal without a runtime sample. It does not choose the list foundation's
template, formal names, compiler-recognized service lowering or primitive storage
interface. That concrete bridge remains
[AMB-006](../../AMBIGUITIES.md#amb-006).

Compiler/runtime primitives remain responsible for every mechanism not expressible
through an agreed ordinary module interface. In particular, any templated list
implementation must preserve the six intrinsic service identities and ephemeral
results in LIST-04, exact acquisition/preflight/refusal in LIST-10, membership and
saved-cursor behavior in LIST-13, and representation/complexity in LIST-03/LIST-05.
An ordinary method body does not automatically provide those service semantics.
Persistent Ama references must remain visible in compiler-managed typed storage;
an opaque native payload must not hide them.

Moving the foundation into Ama requires a lower-level typed storage/lifecycle
interface whose implementation terminates in primitives rather than recursively
depending on the very list it implements. No new allocator interface, unchecked
memory operation, native-template ABI, or replacement of intrinsic list services
with ordinary module instances is introduced here. Such a change requires its
own agreed contract and preservation evidence. See the
[list implementation explanation](lists.md#explanation-templates-and-the-list-implementation)
for the division of responsibilities.

## Compiler obligations and design gates

<a id="tpl-10"></a>

**TPL-10.** Keep parsed definitions immutable. Resolve parameter identities and
scope before structural substitution, retain definition/application source spans,
and construct checked specialization/type information before layout/control-flow
lowering. Report both the relevant definition and application chain for conflicting,
unknown, wrong-category or unresolved bindings. Never change the grammar based
on a successful type lookup.

Use exact static constraint algorithms and canonical specialization caching.
Keep a constructor's nominal origin, fixed-binding map and residual signature
available during checking, including when its use has become a closed Type.
An implementation may intern this metadata; it may not discard information
needed for input-pattern decomposition or nominal identity. Keep capability,
ownership, initialization and execution dependency graphs distinct under COMP-03.
Retain definition/application provenance for the separate specialization-creation
graph under [TPL-11](#tpl-11); a canonical-key cache does not replace its path check.

### Exact structural constraints

For the finite graph of constraints belonging to a supported specialization,
use the following deterministic operations. A static expression keeps references
to scoped parameter identities, rather than copying source names into another
scope:

1. Normalize applications with known origins under TPL-08. Compare ordinary
   nominal origins exactly and decompose equal specialization origins into
   equations for corresponding complete bindings. Unequal origins are a
   conflict, regardless of equal layouts or capabilities.
2. Equate Type variables with Type expressions and Template variables with
   constructors of exactly their declared signature. Respect prior equations;
   reject inconsistent bindings and infinite substitution terms with an occurs
   check. A locally bound residual formal is not a free unknown of its enclosing
   owner. Finite nominal reference cycles are not infinite substitution terms.
3. For an unknown constructor application `F[P]` equated to known nominal static
   information `R = (D, B, S)`, let P be its set of applied formal names,
   B the actual fixed-binding map and S the actual residual signature. Every
   name in P must occur in B. Recover F uniquely as original D with fixed map
   `B minus P` and residual signature `S union P`, using D's original categories
   and nested signatures. This signature must equal F's declared signature.
   Equate each removed B binding with its corresponding explicit right-hand
   expression. Preserve every other fixed binding and every existing residual.
   P consists of source-written bindings normalized to original names and,
   at a shared/body/runtime-result field root, TPL-08's generated bindings for
   omitted residuals. An expression with a static output phase additionally
   uses TPL-12's per-expression inferred producer bindings.
   Generated labels come from the already known signature and use that field's
   deterministic inference-variable identities; they are not guessed formals.
   Without a static output phase, no generated binding is added inside a
   partial static Template argument or static result definition.
   These symbols describe the algorithm, not additional Ama source syntax.
4. In the closed runtime-input case, S is empty, and TPL-07 requires P to cover
   F's complete signature when that input pattern has no static output phase.
   In a nested Template-valued argument without an output phase, S may be
   nonempty: an already partial actual remains partial. A requested P formal
   that is not fixed in the actual cannot be reconstructed by inventing a
   binding; that decomposition fails.
5. If a nominal origin or required equation is not known yet, retain the
   constraint until more declared-path or TPL-03 typed-connection evidence is
   available. Never
   enumerate candidate constructors, partitions of their parameters or possible
   body implementations. Equating unknown constructors does not discover an
   origin. After exhausting these rules unresolved required bindings are an
   underdetermination error. Constant namespaces follow
   [CONST-03/CONST-06](declarations.md#const-06): resolve the concrete namespace
   from independent static information before selecting its declared members.
   Member-name or capability search is not an additional constraint operation.
6. For a static output phase, apply TPL-12: use independently known nominal
   producer origins, expand their declared static result expressions with
   scoped inputs, and add exact output-receiver equations. Check definition
   cycles and TPL-11 creation paths before following each expansion/application
   edge. An output transformation need not be injective; never infer discarded
   inputs or an unknown origin by inventing an inverse. All producer inputs
   must uniquely close before any output is published.

The recovery step solves a declared constructor application under TPL-07 in any
of TPL-03's typed connections. It does not authorize reopening fixed formals
during ordinary static application or treating a supplied concrete Type as a
Template argument. If F already has a binding, check that binding for consistency;
do not replace it with a newly recovered constructor.
Direct static bindings and constraints recovered from typed connections must agree;
neither source has precedence over a conflict. Reordering independent constraints
must preserve the normalized solution or rejection. Multiple unequal concrete
solutions cannot be resolved by picking one.

**Why the recovery is unique.** The actual nominal origin D, its original formal
identities and the declared or generated named set P determine exactly which entries are
removed from B. Both retained bindings and the new residual signature are then
fixed, and the removed entries determine the right-hand equations. No choice
depends on field contents, source traversal order or matching by ordinal. The
same reasoning applies recursively to a constructor-valued binding. Signature
checking rejects a recovered object outside the declared interface.

**Finite-constraint obligation.** Implement equations as a finite structural
constraint graph with shared terms. Bindings only refine unknown information;
they never reopen solved variables or repeatedly manufacture equivalent nodes.
Occurs checking prevents cyclic substitution terms. Each successful refinement
solves/merges a variable or decomposes known finite structure, and an unresolved
pass without a refinement terminates with underdetermination. Provide the actual
algorithm's invariants and progress measure under ARCH-05/ARCH-06. This contract
does not promise arbitrary higher-order unification. The accepted class of
specialization creation and its separate termination argument follow TPL-11.

An occurs check and a cache of repeated specialization keys do not establish that
arbitrary recursive generation terminates: a template can generate a new distinct
specialization at every step. Apply TPL-11's exact creation-path restriction;
do not replace it with an arbitrary nesting limit, timeout, recursion heuristic,
or cache-order-dependent acceptance. Ordinary finite nominal reference cycles
are not automatically infinite type substitutions or eager ownership cycles.

The current sprint's design questions are resolved; the list-foundation rewrite
under AMB-006 remains deferred and outside this boundary. Newly discovered
ambiguities still require an explicit decision before affected implementation.
Supported templates require component and composition evidence covering the
acceptance scenarios, including negative applications. Existing pre-template
acceptance evidence does not establish support for the new grammar or
specialization model. The roadmap and decision registry must keep the distinction
between accepted design and implementation evidence visible.

## Finite composition and specialization creation

**Explanation.** A finite type expression can use the same constructor more than
once. A box of boxes has a perfectly definite type: the outer box receives the
complete inner box type as its element Type. This differs from a definition that
requires another application of itself whenever the compiler expands it.

**Why this rule exists.** Ama is intended to support automatic development and
improvement of algorithms from textual descriptions and examples in other
languages. Nested containers, indexes and reusable wrappers are useful in that
work. Prohibiting every repeated constructor in a type would force artificial
copies of library definitions and expose private helper choices to callers.
The chosen boundary preserves finite composition while excluding recursive
definition expansion, including some cases that would eventually terminate.
It requires no general analysis of whether changing type arguments will stop
growing. It assumes no particular data model for the algorithm-development
polygon.

**Example: complete `.box`.** Assume supplied element types support ordinary
default construction. This definition simply holds a value of its supplied Type:

```text
@<
value
...element type

@!
```

**Example: declaration fragment in an ordinary caller.**

```text
nested
.box
 ...element type < .box
  ...element type < ..integer
```

This is a box whose element type is a box of integers. The two applications are
written in the caller's definition. Neither application is written in `.box`'s
own definition. Its `value` input merely reuses the supplied concrete Type.

**Example: complete `.nest`.** The same composition is also permitted inside
another template, with the deepest element determined by that template's input:

```text
@<
sample&
...element type

@!
nested
.box
 ...element type < .box
  ...element type < ...element type
```

For `.nest` specialized with integers, both box applications originate in that
`.nest` definition. They form two outgoing creation edges, not a box-to-box edge.
Using the resulting inner box Type as the outer box's argument adds no such edge.

**Example: invalid complete `.grow`.** Use a reference field here to separate
specialization creation from eager ownership allocation:

```text
@<
value&
...element type

@!
next&
.grow
 ...element type < .box
  ...element type < ...element type
```

Expanding `.grow` requests `.grow` again with a different element Type. A cache
cannot finish this ever-growing sequence. The reference suffix avoids allocating
the target, but its type still has to exist and pass static checking.

<a id="tpl-11"></a>

**TPL-11.** Permit finite repeated nesting of constructors in static expressions.
Prohibit repetition of an original source template on any path of the
specialization-creation graph defined below, even when the repeated application
has exactly the same complete bindings. This restriction applies to required
specializations before code generation or runtime construction.

### Construction provenance

The graph distinguishes concrete module types by their full nominal identities
under TPL-05, including all normalized Type/Template bindings. An ordinary module
definition can also be an owner of applications. It has its ordinary concrete
identity rather than an invented template parameter or default specialization.

For each required concrete owner, record an edge to the concrete specialization
required by each constructor application originating in that owner's instantiated
definition. Include applications in shared declarations, input/runtime-result
types, static result definitions and projections under TPL-12, defaults, body
declarations and explicit constant namespaces. A projection's concrete producer
is required even when its ordinary application Type is discarded. Returning a
supplied Type adds no application edge; a later application of a returned
Template retains its actual original constructor and captures. Include applications
inside other applications' static arguments, and applications introduced or
completed by the permitted inference rules. Keep the originating definition and
application source locations on the edge. Runtime branch reachability does not
remove these static requirements.

Every application inside a static expression belongs to that expression's
lexical definition owner. Syntactic argument nesting alone does not add an edge
from the outer application's result to the inner application's result. For
example, if owner `O` writes `Box(Box(Integer))`, record edges from `O` to each
box specialization. Add no outer-box-to-inner-box creation edge unless the box
definition itself contains a corresponding constructor application.

Merely using a supplied concrete Type, or decomposing its known nominal metadata
under TPL-10, creates no application edge. Validate that Type and its required
definition independently; this is not permission to leave an argument type
unchecked. A Type parameter cannot be applied as a constructor under TPL-02.
An explicit static output phase is the required producer application defined
by TPL-12, rather than this plain reuse of supplied nominal metadata.

For a Template parameter, use the actual original constructor and its fixed
bindings, not the formal's name or signature alone. A partial constructor remains
finite static metadata under TPL-08/TPL-10. Capturing or forwarding an unapplied
constructor, and partially applying a head that remains open, do not themselves
require a specialization of that head. Any closed applications in their written
argument expressions are still accounted for. When the constructor is fully
applied, record the application with its actual origin and retained captures,
including bindings unused by generated code.

The initial requirements arise from finite source expressions in the required
ordinary definitions and selected concrete compilation roots. Every additionally
required specialization must be attributable to such a root or to a recorded
application edge. Inferring a binding cannot manufacture unrelated type origins
or exempt an implicit application from this provenance requirement.

### Path checking and canonical caching

On every creation-edge path, compare original full logical source-template
identities. Reject the first repetition on that path, regardless of unequal
bindings, equal canonical keys, aliases, partial-binding routes or generated
names. Direct self-application and recurrence through other templates are both
invalid. No exception is made for reference storage, an initially empty list,
finite permutations of bindings or a recursive definition that would terminate.

Check independent paths independently. Keep different concrete specializations
as different graph vertices; merging their edges into one graph of source names
can introduce cycles that no actual creation path contains. Repeated sibling
applications and repeated origins in the structure of supplied Types are not
themselves a repeated origin on a creation path.

Record an application edge even when its result is already interned or checked.
A cache hit cannot authorize a repeated origin. When a cached dependency subgraph
is used under a new ancestry, check its outgoing paths against that ancestry too;
checking only the immediate cached target is insufficient. For equivalent
normalized bindings, changing source discovery order, reordering independent
named bindings or using a warm instead of cold cache must yield the same
acceptance or rejection. An implementation may retain exact
dependency summaries instead of repeating body work, provided the same path
predicate is preserved.

Report a rejected path with the applications and original source identity that
repeats, retaining definition and use locations under TPL-10. Do not diagnose a
finite repeated type expression as recursion merely because its printed type
name contains an earlier constructor name.

### Relationship to the other checks

Creation paths are separate from the structure of specialization bindings, module
loading, eager ownership, constant initialization, clone capability and runtime
execution. Continue checking finite signatures under TPL-02, finite substitution
terms under TPL-10 and eager construction under
[TYPE-04](declarations.md#type-04). Ordinary finite nominal reference cycles and
reuse of an already supplied Type remain governed by their own rules; they do
not imply a new constructor application. Runtime calls keep their ordinary
instance availability rules and do not generate types during execution.

**Examples: explanatory metanotation, not additional Ama syntax.** In this table
an arrow means a constructor application originating in the expanded definition:

| Case | Required result |
|---|---|
| `Box(Box(Integer))`, where Box only uses its supplied Type | Permitted, including when written inside another template |
| `Pair(Box(Integer), Box(Text))` | Permitted independent branches |
| `A(T) -> A(T)` | Rejected even for identical full bindings |
| `A(T) -> B(T) -> A(T)` | Rejected mutual recurrence |
| `Grow(T) -> Grow(Box(T))` | Rejected changing bindings |
| `A(X,Y) -> A(Y,X)` | Rejected finite permutation recurrence |
| An actual Template parameter applies a partial constructor originating in its owner | Rejected repeated source, even if its next application would terminate |
| A constructor is captured but never applied | No creation edge for that capture; its finite metadata is still checked |

**Explanation: private helpers do not prohibit finite composition.** Suppose
`Set(T)` is implemented using `Dictionary(T, Marker)`, and Dictionary only reuses
its supplied key/value Types. An Index definition requiring
`Dictionary(Feature, Set(Node))` has paths `Index -> Dictionary` and
`Index -> Set -> Dictionary`. The supplied Set Type adds no `Dictionary -> Set`
edge. This composition is permitted even though Dictionary occurs twice in the
complete type structure. If an actual definition additionally creates a return
path to an earlier source template, that different construction is rejected.

**Explanation: unrelated bindings must not create a fictitious cycle.** Suppose
A and B each apply a received one-Type constructor to their element Type. One
root supplies A with B partially fixed to use Box; its creation path is
`A -> B -> Box`. Another root supplies B with A partially fixed to use Box;
its path is `B -> A -> Box`. These different concrete applications are permitted.
Combining their edges by source name would falsely produce `A -> B -> A`.

**Why checking terminates.** Source definitions, their static expressions and
their initially required roots are finite. Each instantiated definition contains
finitely many application occurrences. The path predicate bounds the number of
template-source steps by the number of original templates, so finite branching
cannot produce an infinite creation tree. Reuse of supplied Types adds no new
application step; all required concrete Types still have their root/application
provenance. Constructor captures and constraints remain finite under the separate
signature and occurs checks. This is a structural termination argument, not a
configured depth limit: explicit finite nesting has no additional language depth
limit, and no timeout or heuristic substitutes for the path check.
