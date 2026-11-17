# Static results and positional application

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a library can provide types](#explanation-a-library-can-provide-types)
- [Defining static results](#defining-static-results)
- [Selecting a producer and its results](#selecting-a-producer-and-its-results)
- [Named and positional interfaces](#named-and-positional-interfaces)
- [Receiving an immutable static name](#receiving-an-immutable-static-name)
- [Inference and complete producers](#inference-and-complete-producers)
- [Composition and specialization identity](#composition-and-specialization-identity)
- [Dependencies, checking, and implementation](#dependencies-checking-and-implementation)
- [Examples of rejected programs](#examples-of-rejected-programs)

<!-- /chapter-contents -->

## Explanation: a library can provide types

A module library can provide a concrete type, or a constructor that still needs
some static arguments. For example, an algorithm can fix a dictionary's key
type and provide a family of dictionaries whose value type remains selectable.
Clients reuse that family instead of repeating the key binding at every site.

There are two distinct activities. The compiler substitutes types and checks
their operations. The running program creates instances, stores values and calls
methods on those instances. A static result belongs to the first activity. A
runtime result such as `get` belongs to the second.

**Design rationale.** Named static results make related types part of a library
interface. Partial application supplies the returned constructor without a
separate type-lambda notation. The two-line declaration keeps the familiar Ama
shape, while its leading `...` marks a static name. Restricting definitions to
structural static expressions permits exact substitution and the existing
finite-specialization checks, without executing arbitrary Ama or native code
during compilation.

## Defining static results

<a id="tpl-12"></a>

**TPL-12.** A module may define static Type or Template results in `@>` and
select them through explicit static output bindings. Static formals, results,
applications and receivers obey the rules in this chapter together with
[TPL-02/TPL-07/TPL-08](templates.md#tpl-02). Every static name uses TPL-01's
complete placeholder spelling; bare `...` is solely a positional binding marker.

A static result has exactly two same-level head lines: its name, then its
defining static expression. Arguments of that expression are one indentation
level deeper. The definition has no runtime storage suffix, literal, ordinary
initializer, input default or mutable result cell. It contributes no runtime
input/output position, acquisition, reset, copy or continuation state.

**Example: complete `.prepare dictionaries`.** Assume the supplied family has
exactly the two Type formals shown, and supports the operations required by any
later runtime use:

```text
@<
...key type

...dictionary family
 ...key type
 ...value type

@>
...dictionaries for key
...dictionary family
 ...key type < ...key type

...integer dictionary
...dictionary family
 ...key type < ...key type
 ...value type < ..integer

@!
```

The first result is Template with only `...value type` residual. The second is
Type. The empty implementation is sufficient because these definitions are
static; a client does not call this module's body to obtain them.

The second line defines the static result's value, rather than the type of a
runtime cell. Determine the result category and, for Template, its exact
residual signature from that expression and the declared parameter signatures.
An application without a static output phase preserves omitted residuals here
as in a static argument expression under TPL-08. It does not introduce runtime
field inference variables. Captured producer parameters and a returned
constructor's own residual formals remain distinct.

Collect all explicit static result names before interpreting interface
expressions. They are bound names, not new free parameters of their producer.
Each is visible throughout its declaring module, including other interface
expressions and earlier text, subject to the dependency checks below. A result
name must be distinct from every static formal and other explicit static result
name of that module. Runtime names occupy their separate namespace.
Genuinely undeclared bare Type names in permitted interface expressions may
still introduce producer Type parameters under TPL-02; a reference to a
collected result name never does. New Template parameters require an input
declaration/signature under TPL-07.

Runtime and static results may be interleaved in `@>`. Ordinary `>`/`>>` reads
and CALL-09 selectors use only the runtime interface. Static output bindings
use only the explicit static result definitions. A runtime result's type does
not itself declare a static result.

## Selecting a producer and its results

A standalone static projection has a static-expression head followed by zero
or more static input bindings and at least one static output binding. Its head
is a concrete module path or a visible Type/Template name. It has no `!`, `%`
or refusal handler. The whole construct contributes compile-time constraints;
its source position in a runtime body does not make it execute at runtime.

Static output bindings may also occur after the static inputs of an expression
inside a declaration, static result definition, argument or constant namespace.
They select that expression's producer outputs. They do not replace the
expression's own application value with the selected result: after resolution,
the expression still denotes the producer's fully determined Type. A standalone
projection uses its static outputs and need not store that producer Type.

The presence of any static output binding, including a positional skip,
requests a fully determined producer specialization. After matching explicit
input bindings, give each omitted producer formal a distinct inference term of
its declared category/signature. Identify it by the enclosing specialization,
the expression's lexical source node and the original residual formal identity.
For a Template-valued formal, a determined binding is a known original
constructor, fixed captures and exact residual signature; its own locally
bound residual formals need not be saturated.

Solve the producer's applicable static constraints together with the output
equations below. Before publishing any selected result, every producer input
binding must be uniquely determined under its enclosing specialization. This
includes inputs not mentioned by a selected output. Validate the complete
required producer specialization under TPL-06. The producer's body, defaults,
native hooks and runtime initializers do not execute.

Without a static output phase, omission retains TPL-07/TPL-08's existing
context-specific meaning: partial static arguments/results retain residuals,
runtime declaration roots infer field bindings, ordinary root input patterns
require coverage, and a constant namespace requires independent closure.
An output phase provides the same producer-inference rule in all expression
contexts; it never adds a free parameter to the containing module.

Determine the producer's actual nominal origin from explicit paths, supplied
static metadata or permitted nominal constraints before looking up outputs.
Then resolve named outputs or positions in that definition's explicit static
`@>` interface. A Template parameter's input signature does not promise particular
outputs: check the actual supplied constructor's output declarations when its
origin is known, as for other operations on supplied types under TPL-08.
Retain a pending constraint while independent evidence may determine that origin;
report underdetermination if it never does. An output name, requested category,
receiver type or matching body is not evidence for searching candidate modules
or choosing an otherwise unknown origin.
Once the producer origin is known, an output expression that contains a
declared constructor-application pattern may still recover an input constructor
through TPL-10's exact nominal decomposition. This is structural inference
within that known definition, rather than a search for a producer.

## Named and positional interfaces

A named input is `...formal < expression`. A named output is
`...result > ...receiver`. The left name belongs to the selected producer's
interface; sources and receivers belong to the supplying module. Names match
exactly. Unknown or repeated named labels are errors, including equal repeated
bindings. A named binding cannot omit its source or receiver.

Positional bindings use bare `...` instead of a foreign label:

```text
... < ..integer
... <
... >
... > ...received type
```

Each line consumes one position in its respective sequence. Empty `... <`
does not supply a binding. Empty `... >` does not create an alias or receive a
result. An omitted trailing binding consumes no additional position. Every
explicit skip counts toward arity; positions beyond the interface are errors.

Take the input sequence from TPL-02's original formal order of the actual
constructor, restricted to its residual formals at the start of this application.
It includes explicit and interface-introduced producer parameters in that
order; a parameter first introduced through a runtime result type remains a
producer parameter. Captured/fixed formals are excluded. Snapshot this sequence
before processing any binding: filling the first slot does not renumber the
second. Expected signatures remain named, order-insensitive contracts under
TPL-07; writing a supplied constructor's expected signature in another order
does not reorder its actual positional inputs. Delay positional resolution
until the actual origin is known. Positions alone cannot introduce an unknown
Template's named signature.

The output sequence is exactly the explicit static result declarations in
their textual order in the actual producer's `@>` section. Runtime fields,
free parameter introductions, binding lines and signature children contribute
no static output positions. The ordinary runtime sequences separately exclude
all static declarations.

Input and output counters are independent. All input bindings precede every
output binding at their expression level. Use either named mode throughout
that level's inputs and outputs, or positional mode throughout; mixing modes
is an error. Nested expressions and ordinary runtime operations choose their
own modes independently. All separators and indentation follow SYN-06.

**Example: caller body fragment.** Assume `.dictionary` has the two explicit
Type inputs `...key type` then `...value type`, and its concrete instances
support ordinary default construction:

```text
.prepare dictionaries
 ... < ..text
 ... < .dictionary
 ... > ...text key dictionaries
 ... > ...text integer dictionary

first
...text integer dictionary

second
...text key dictionaries
 ...value type < ..integer
```

Both fields have the same dictionary Type, with independent runtime storage.
An equivalent named projection is:

```text
.prepare dictionaries
 ...key type < ..text
 ...dictionary family < .dictionary
 ...dictionaries for key > ...text key dictionaries
 ...integer dictionary > ...text integer dictionary
```

These are alternative caller fragments. To skip the family and receive only
the concrete Type, use:

```text
.prepare dictionaries
 ... < ..text
 ... < .dictionary
 ... >
 ... > ...text integer dictionary
```

Skipping an output suppresses only its binding to the caller. It neither makes
an invalid producer valid nor skips checking declared static result definitions.

## Receiving an immutable static name

Resolve a nonempty receiver as a complete name in the supplying static scope.
If it already denotes a parameter, static result or visible alias, impose exact
equality with the selected output. A Type must agree in nominal identity; a
Template must agree in original constructor, fixed bindings/captures and exact
residual signature. An unresolved parameter may be constrained by that equality;
an already determined name is never reassigned or reopened.

If the receiver is new, introduce an immutable alias for that selected Type or
Template. Its category and metadata come from the result; the alias contributes
no independent concrete type evidence. It creates no runtime field and no free
parameter or specialization-key component of the containing module.

For receiver classification inside interface expressions, first collect explicit
input formals and static result binders, whose scope is the whole module. For
each remaining name, compare receiver positions with eligible implicit
interface-parameter introduction positions in lexical order. A receiver before
any such introduction establishes an alias; its subsequent in-scope uses are
uses of that alias, not later implicit parameter introductions. Otherwise the
receiver constrains the interface parameter already introduced. Finish the
whole-interface category/signature collection under TPL-02 on those genuine
parameter sites. This preserves forward visibility of owner parameters without
turning a newly received alias into an owner parameter when it is later used as
an interface type. An earlier unbound body source remains an error.

The alias is visible at its receiver and subsequent source positions through
the remainder of the containing module, including nested blocks. Ordinary
branch, loop, foreach and refusal-handler nesting does not shorten this static
scope. A later receiver with the same visible name imposes equality instead of
shadowing it. An earlier unresolved use cannot be repaired by a later alias.
The alias's name does not become a producer formal, a foreign binding label or
a dotted member-access prefix.
Its own producer and input expressions cannot use the new receiver as an
earlier source. Nested projections introduce aliases at their actual receiver
positions; later expressions may use them, while foreign input labels never
introduce names into the supplying scope.

Classify static names by source position independently of runtime control flow.
An alias received inside a runtime branch is established statically even when
that branch is not taken; incompatible equations in different branches are
errors. All ordinary runtime ownership, initialization and scope rules remain
applicable to fields typed through such an alias.

**Example: body fragment with a known producer interface.**

```text
.prepare dictionaries
 ... < ..text
 ... < .dictionary
 ... > ...text key dictionaries

worker
.typed consumer
 ...received family < ...text key dictionaries
```

Here `.typed consumer` is the TPL-08 consumer whose expected family has only
`...value type` residual. Its ordinary typed input calls must still determine
the element Type and satisfy that family's actual runtime interface. Receiving
the family does not create or initialize a dictionary instance.

## Inference and complete producers

Static output definitions add structural equality constraints to TPL-10.
Substitute the producer's explicit and inferred bindings into the selected
definition, preserving scoped identities. An existing determined receiver may
then constrain producer inputs visible in that expression. A fresh receiver
merely names the output expression; it does not choose its unknown bindings.
Preserve every producer input in its specialization key, even if its value is
discarded by all output definitions.

**Example: complete `.type identity`.**

```text
@<
...type

@>
...same type
...type

@!
```

After the earlier caller has established `...text integer dictionary`, this
projection determines the identity producer's omitted input:

```text
.type identity
 ... > ...text integer dictionary
```

Its input is that exact dictionary Type. In contrast, the same projection into
a new `...unknown type` leaves both its producer input and output undetermined
and fails. A positional skip instead of the receiver supplies no constraint.

**Explanation: information can be lost.** In mathematical notation, not Ama
syntax, let `Factory(K, V)` return `List(V)`. An expected `List(Integer)` can
determine `V`; it contains no information about `K`. Supply `K` or determine it
through another permitted constraint. If a factory always returns `Integer`,
that result alone determines none of its input parameters. The compiler never
invents an inverse or chooses an unused argument.

Full producer determination and a returned Template's residual signature are
different requirements. `.prepare dictionaries` has its own key Type and family
argument completely determined above. Its `...dictionaries for key` result
deliberately retains the returned dictionary constructor's value formal. This
is a valid closed producer returning a partial constructor.

## Composition and specialization identity

A received Type can type runtime storage or serve as a Type argument. A received
Template can be forwarded to a compatible Template formal, partially applied
again or completed into Type. Apply TPL-08's exact named capture and residual
rules at each step. Parameter names in separate modules never connect by
spelling; binding labels and source expressions establish the connection.

Static result aliases do not manufacture new nominal types. Two producers
returning the same `.dictionary` with the same complete bindings return the
same Type, even when their own producer specializations differ. The result
retains the returned constructor's origin, not a new origin formed from the
producer name or the alias. Equal partial constructors retain equal metadata.
Different producers still have their own complete identities and required
checks. Multiple runtime instances of one returned Type remain independent.

**Example: caller fragment.** Assume `.wrap type` has one Type input and returns
its `.box` specialization through one static result:

```text
.wrap type
 ... < ...text integer dictionary
 ... > ...boxed dictionary

boxed
...boxed dictionary
```

The first producer's concrete dictionary result becomes the second producer's
input. A further producer could receive `...text key dictionaries` as a Template
instead. Neither case requires a temporary runtime instance of the producer.

## Dependencies, checking, and implementation

Check every declared static output of each required concrete producer, including
unselected and positionally skipped outputs. Check normal type, operation,
lifecycle, shared-constant and ownership obligations under TPL-06. Static output
expressions cannot branch on runtime values, inspect list contents or length,
execute module bodies/native hooks, or depend on method-initialization flags.
Numeric static arguments and arbitrary compile-time Ama execution are outside
these Type/Template source forms.

Keep the graph of static value-definition expansion separate from ordinary
type-reference, constraint-equality, ownership and constant-value graphs. Its
nodes identify a static result or receiver-alias definition in its scoped
producer/application environment. Add an edge for a definition that requires
another static definition's expansion, including selected foreign static
results. Reject directed definition cycles with the relevant definition and
application locations. Merely mentioning the type of an ordinary reference
field is not such an edge. TPL-10's occurs check and its permitted anchored
equality cycles remain separate from definitional expansion cycles.

Retain TPL-11's specialization-creation provenance. A static output phase creates
a required concrete producer application even if the caller discards the normal
producer Type. Attribute that application to its enclosing instantiated
definition. This also applies when the head is an already supplied closed Type
or an ordinary module with no static parameters: reading its static results
requires checking its producer definition, rather than merely decomposing type
metadata. Preserve ordinary producer identities in dependency records; TPL-11's
repeated-template predicate concerns original template origins. A concrete constructor application written inside the producer's
output definition is an edge from that producer; returning a supplied Type
adds no fictitious application. A returned partial constructor is finite
metadata until its later application, which uses its actual original origin
and captures. Output aliases and warm cache entries cannot conceal a repeated
origin on an actual creation path.

During inference, retain distinct application occurrences and their scoped
symbolic input environments while complete canonical keys are unresolved.
Check already known original template origins on the active creation ancestry
before expanding their definitions. Finalize and cache concrete vertices once
bindings close; merging equal keys must preserve every incoming provenance and
its path validation. This allows output equations to determine inputs without
first expanding an unguarded recursive producer chain.

An exact checker can proceed as follows:

1. Parse immutable syntax. Collect explicit interface formals and output
   binders separately; classify received aliases and genuine implicit parameter
   sites in lexical source order, then finish whole-interface parameter
   category/signature collection under TPL-02.
2. Create scoped terms for producer applications, their omitted input bindings
   when an output phase is present, and each static output equation. Normalize
   positional labels to actual original formal/result identities once that
   origin is independently known. Retain source locations and positional skips.
3. Apply TPL-10's exact structural equations. Expand known static result
   definitions without losing parameter scope. Before following each
   definition/application dependency, check the active static-definition path
   and TPL-11's concrete creation ancestry; reject a repeated forbidden origin
   before expanding its next definition. Cache hits retain these path checks.
   Do not invert a general output
   transformation, enumerate constructors, infer from capabilities, or choose
   a binding from execution order. A blocked expression waits only for actual
   new static evidence; an iteration without a change is not progress.
4. Require unique complete producer bindings, finish validating the already
   guarded static-definition graph and TPL-11 creation paths, and check all required specialized
   definitions. Reject unresolved required origins, inputs or captures.
5. Publish the normalized result aliases and concrete runtime types. Retain
   provenance for diagnostics and cache canonical specialization information.
   Lower only the resulting ordinary runtime operations and storage.

**Correctness rationale.** Positional normalization maps each consumed ordinal
to exactly one original interface identity; named and positional forms therefore
produce the same equations when they select the same slots. Substitution follows
scoped identities, so no name capture or source-text replacement occurs. Acyclic
static definitions admit dependency-ordered expansion. TPL-11 separately bounds
required specialization creation; partial output metadata does not introduce
an unchecked expansion mechanism. Exact equality solving and closure establish
one normalized producer and result identity, or report a conflict or missing
evidence. These obligations provide the finite structural basis for checking;
tests additionally exercise the boundaries and real component handoffs.

## Examples of rejected programs

**Examples: fragments with the interfaces above assumed.**

| Case | Required diagnosis |
|---|---|
| `@>` contains a lone `...result` with no expression | Incomplete static result declaration |
| Give a static result `&`, `[]`, a literal or ordinary value initializer | Invalid static result source form |
| Define a result with the same complete name as a static formal | Duplicate static declaration |
| Use a later receiver alias in an earlier expression | Out-of-scope static name |
| Read a runtime `get` field as a static output | Not a declared static output |
| Use a static result in an ordinary `>` or CALL-09 selector | Not a runtime result |
| Mix named input with positional output at one level | Mixed static binding modes |
| Supply input after a static output or output skip | Invalid static phase order |
| Consume a third output of `.prepare dictionaries` | Excess static output position |
| Rebind the fixed key of its returned family | Nonresidual input binding |
| Receive a different determined Type into an existing alias | Conflicting immutable static equality |
| Receive `.type identity` output only into a fresh alias | Underdetermined producer input |
| Leave a producer input unused by its otherwise concrete output unknown | Underdetermined producer input, even when the selected result is known |
| Select an output of an origin that has never been determined | Underdetermined producer origin; no member search |
| Define `...a` through `...b` and `...b` through `...a` | Cyclic static value definition |
| Return an invalid constructor application only in an unselected output | Invalid required producer specialization |
| Choose a static output from a runtime `initialized` flag | Runtime-dependent static definition |

The source forms are canonical requirements, not evidence of compiler support.
Acceptance cases are listed in the
[template conformance matrix](../implementation/tools-and-conformance.md#static-module-templates).
