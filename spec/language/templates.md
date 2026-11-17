# Static module templates

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: one definition, several concrete types](#explanation-one-definition-several-concrete-types)
- [Placeholder spelling](#placeholder-spelling)
- [Parameters and their scope](#parameters-and-their-scope)
- [Inferring types from ordinary arguments](#inferring-types-from-ordinary-arguments)
- [Nested template instances](#nested-template-instances)
- [Specialization identity and independent state](#specialization-identity-and-independent-state)
- [Checking a specialization before execution](#checking-a-specialization-before-execution)
- [Type parameters and template parameters](#type-parameters-and-template-parameters)
- [Partial substitution and composition](#partial-substitution-and-composition)
- [Templates and Ama's own lists](#templates-and-amas-own-lists)
- [Compiler obligations and design gates](#compiler-obligations-and-design-gates)

<!-- /chapter-contents -->

## Explanation: one definition, several concrete types

A template is a module definition with named static parameters. A placeholder
stands for a module type. The compiler obtains concrete types from typed values
connected to the template, substitutes them into the program structure, and checks
the resulting module before generating executable code. Runtime values and
execution state remain ordinary Ama values and state.

For example, a queue can store different types of work without maintaining a
separate handwritten queue definition for each work type. A template may also
own a field of another template's type. That field has its own inferred static
bindings and its own runtime state.

**Design status.** The placeholder spelling, ordinary-call model, nested-field
model and identity contracts below record the agreed design. Template-valued
parameters and partial substitution also have the semantic contracts below;
their declaration/application notation is not yet settled. The
[open-question index](../../AMBIGUITIES.md#open-template-design-questions) is the
single place to find pending decisions, including inference boundaries, recursive
specialization, native templates and the list implementation bridge. This chapter
does not imply that the compiler implements templates. Do not invent source forms
or implement a pending case by guessing an answer. The grammar recognizes only
the source forms explicitly specified in [the grammar](../grammar.md).

## Placeholder spelling

<a id="tpl-01"></a>

**TPL-01.** A placeholder type consists of exactly three adjoining ASCII dots
followed by one or more complete name components separated by single adjoining
dots. Each component follows [SRC-03](source-and-names.md#src-03), including its
Unicode, underscore and single-space rules. There is no terminating dot.

The whole spelling is one parameter name. In `...data.element type`, neither
`data` nor `element type` denotes a separately addressable parameter or field.
The components explain the parameter's purpose. They are not a physical path,
an ordinary/standard-library root selector, or an instruction to find a file.
No source with that name is required or loaded. A component may use any spelling
allowed by SRC-03 without an existing directory or module of that name.

Recognize `...` before the concrete-path prefixes `..` and `.`. The prefix
directly adjoins its first component; internal dots directly adjoin both
components. Empty components, bare `...`, four leading dots, trailing dots,
internal doubled dots and spaces adjoining a dot are invalid. Indentation and
the separator outside an operand remain outside the placeholder name.

A placeholder is permitted only in an explicitly defined type position. It is
not a value source, runtime variable, call target, constant namespace, index,
literal, or syntax for selecting a member of a type parameter. In particular,
`...type.field` never means access to `field` through `...type`.

**Example: spellings, not complete modules.**

| Spelling | Interpretation |
|---|---|
| `...data.element type` | One parameter with two descriptive name components |
| `...data.key type` | A different parameter, despite the shared first component |
| `...имя типа.назначение` | The same naming rules with international words |
| `...data .element` | Invalid space before a separator |
| `...data.element.` | Invalid terminating dot |

## Parameters and their scope

<a id="tpl-02"></a>

**TPL-02.** A placeholder's occurrence as the type of an input declaration in
`@<` introduces a type parameter of that module template. Collect these
introductions from the whole input section. Repeated occurrences of the same
complete placeholder name refer to the same parameter; distinct complete names
introduce independent parameters. Declaration order for parameter identity is
the order of their first occurrences in `@<`.

The parameter identity comprises its defining module identity and complete
placeholder name. Identical spelling in another template introduces a different
parameter. Physical submodule nesting does not inherit parameters under
[SRC-05](source-and-names.md#src-05). Ordinary variables and type parameters have
different roles: the variable holds runtime state; its placeholder describes
the module type of that state.

Type positions in `@>` and the Ama body may reuse only parameters introduced
through that template's `@<`. A new placeholder appearing only in a result or
body is a static error, including in an unreachable branch or empty-loop body.
The body cannot add implicit type parameters to its enclosing module. Shared
`@=` declarations do not introduce type parameters; dependent uses there belong
to the explicitly pending inference/constant-access decision in
[AMB-004](../../AMBIGUITIES.md#amb-004), rather than an inferred extension of scope.

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
in a result or body type line is valid; introducing `...data.result type` only
there is invalid.

## Inferring types from ordinary arguments

<a id="tpl-03"></a>

**TPL-03.** An ordinary entry call keeps the existing named/positional argument
syntax. The caller passes values, references or lists under
[CALL-01/CALL-02](calls-and-results.md#call-01). A typed actual input constrains
the corresponding declared placeholder to that actual's module type. Reference
and list arguments contribute their statically known target/element type;
their ownership and scalar/list compatibility are checked separately under
[TYPE-05](declarations.md#type-05).

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

Known types are evidence even when a list is empty or a reference has no live
target. Their contents/liveness still follow the ordinary runtime rules. Literal
bytes such as `|0` have no intrinsic module type and cannot select one by their
spelling. A discarded result, an untyped receiver introduced by TYPE-06, or an
equation between unknown types alone supplies no concrete type identity.

The settled input-based inference applies to direct calls and calls of selected
results under CALL-09, with each operand's ordinary lexical environment. The
additional roles of typed defaults, declaration initializers, aliases,
assignments, typed output receivers and dependent constants are explicitly open
in [AMB-004](../../AMBIGUITIES.md#amb-004). Do not silently accept or reject one
of those unresolved cases by adding an inference heuristic. Required concrete
bindings must ultimately be unique; missing evidence cannot be repaired with a
default type or an implicit instance.

## Nested template instances

<a id="tpl-04"></a>

**TPL-04.** A template may explicitly declare fields whose type path names
another template. No special nested-call syntax is required. The nested
template introduces its parameters in its own `@<`; those declarations do not
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
capabilities. Defining the template's implementation itself using `@C`, selecting
generic exported roots and supplying specialization-qualified external constant
access require the unsettled contracts linked from
[AMB-007](../../AMBIGUITIES.md#amb-007) and
[AMB-004](../../AMBIGUITIES.md#amb-004). No implicit native layout, C cast, export
selection or default specialization is permitted to fill that gap.

The introduction-only-in-inputs rule also constrains algorithms: a general
conversion with distinct input/result types must introduce both independent type
parameters through permitted inputs, or use a separately agreed constructor
signature mechanism. A body-inferred result variable does not introduce a new
placeholder name into `@>`. Empty generic containers likewise need a statically
typed source of their bindings; missing runtime elements provide no evidence.

## Type parameters and template parameters

<a id="tpl-07"></a>

**TPL-07.** Distinguish two static parameter categories:

| Category | Meaning of its binding |
|---|---|
| Type parameter | One module type, concrete or symbolically dependent on a parameter in an enclosing composition |
| Template parameter | A named template constructor with an expected named parameter signature, possibly carrying prior partial bindings |

For a template parameter the expected signature must identify the remaining
formal parameter names and their categories, recursively including expected
signatures for template-valued formals. Matching must be explicit and
scope-aware. A coincidentally equal spelling in two definitions is not a binding
or permission to rename a formal. Unknown names, duplicate bindings, incompatible
categories and incompatible expected signatures are static errors.

An actual concrete instance identifies its own type, not a unique choice of
which constructor parameters should remain open. Recovering a constructor from
such a type is permitted only under an explicitly defined signature/binding
rule that yields a unique result. Do not search available modules for a plausible
constructor, compare bodies/layouts, or silently choose which binding to erase.

**Example: the ambiguity is static, not runtime.** A dictionary with text keys
and integer values could supply either a constructor retaining text keys and
leaving its value parameter open, or one retaining integer values and leaving
its key parameter open. The receiving signature must distinguish those requests.
An ordinary argument alone does not express that distinction.

These requirements do not require new punctuation or explicit type arguments at
ordinary call sites. The representation of categories, expected signatures and
named static bindings remains [AMB-003](../../AMBIGUITIES.md#amb-003). No new
annotation, interpretation of compound placeholder components, or constructor
application production is established by an explanatory example.

## Partial substitution and composition

<a id="tpl-08"></a>

**TPL-08.** Static substitution operates on bound program/type structure, never
on source text. It replaces references to specific formal parameter identities
in their lexical environment. It preserves caller/callee scope and cannot capture
an unrelated parameter or variable with the same spelling.

A partial substitution binds a subset of a constructor's parameters. The result
retains the original constructor identity, normalized supplied bindings, captured
lexical dependencies and an explicit signature for the unbound parameters. It
remains a template, can receive further explicit bindings and can serve as a
template argument under TPL-07. A parameter may bind to another type parameter;
the resulting dependency remains symbolic until that parameter is bound.

An outer template may receive a constructor, supply its own parameters to that
constructor and use the resulting type/template. Named bindings refer to the
selected constructor's own formal identities; the supplied expressions refer to
the explicit supplying environment. Identical names never create connections
by themselves. Nested ordinary calls in TPL-04 already establish symbolic type
equalities without additional call syntax; independently passing a residual
constructor additionally requires TPL-07's signature contract.

**Example: successive binding, expressed in words rather than Ama syntax.** A
dictionary constructor starts with key-type and value-type parameters. Binding
only key-type to text produces a constructor whose remaining parameter is
value-type. Binding that parameter to integer produces the same specialization
as binding both original parameters to text/integer in one step. Two objects
created with that specialization still have independent dictionaries.

Residual formals and captured dependencies are different. A dictionary whose
key is bound to an outer parameter still depends on that parameter even after
its value formal is bound. Conversely, a template-valued argument may retain
its own declared formals without making the receiving outer specialization open:
those formals are locally bound constructor parameters, not unresolved free
variables. Every type actually used as executable storage must nevertheless be
concrete under TPL-06.

Omission of a runtime `<` binding, an empty positional `<`, `>` or a literal is
not a partial static application. Those forms retain their existing call/default
semantics. An open constructor is not an Ama runtime result, mutable field,
checked reference or dynamically replaceable type. Its source-level declaration,
transmission and application remain gated by AMB-003.

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
that type. The bridge supplying that type to an ordinary template whose formals
are introduced through `@<` is a separate pending design decision,
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
Within a finite first-order constraint graph, unify full nominal type identities
and scoped parameter variables; reject inconsistent equations and infinite
substitution terms with an occurs check. Equating two unknowns does not produce
a concrete type. Keep capability, ownership, initialization and execution
dependency graphs distinct under COMP-03.

An occurs check and a cache of repeated specialization keys do not establish that
arbitrary recursive generation terminates: a template can generate a new distinct
specialization at every step. The permitted recursive-specialization class and
its exact terminating analysis remain [AMB-005](../../AMBIGUITIES.md#amb-005).
Do not replace that decision with an arbitrary nesting limit, timeout, recursion
heuristic, or cache-order-dependent acceptance. Ordinary finite nominal reference
cycles are not automatically infinite type substitutions or eager ownership cycles.

Before implementing a pending case, resolve its linked question and update the
authorized canonical rule/grammar. Supported templates require component and
composition evidence covering the acceptance scenarios, including negative
applications. Existing pre-template acceptance evidence does not establish
support for the new grammar or specialization model. The roadmap and open-question
registry must keep this implementation gap visible.
