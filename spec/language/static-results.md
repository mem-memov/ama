# Static modules and exported types

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: assembling types before execution](#explanation-assembling-types-before-execution)
- [The static module](#the-static-module)
- [Static type markers](#static-type-markers)
- [Defining and forwarding a result](#defining-and-forwarding-a-result)
- [Calling a static producer](#calling-a-static-producer)
- [Inputs, public names and scope](#inputs-public-names-and-scope)
- [Requirements inferred from uses](#requirements-inferred-from-uses)
- [Inference and complete producers](#inference-and-complete-producers)
- [Using an exported Type in ordinary code](#using-an-exported-type-in-ordinary-code)
- [Composition and specialization identity](#composition-and-specialization-identity)
- [Separate graphs and cycle checks](#separate-graphs-and-cycle-checks)
- [Checking and implementation](#checking-and-implementation)
- [Examples of rejected programs](#examples-of-rejected-programs)

<!-- /chapter-contents -->

## Explanation: assembling types before execution

A static module describes how types fit together. It can fix a dictionary's key
type, return a family of dictionaries whose value type remains open, or supply
several related concrete types to a program. The compiler solves these
relationships before generating executable code. Its purpose is to generate
concrete ordinary module types from reusable definitions, instead of requiring
the programmer to write a separate source module for every needed combination.
Static modules assemble those definitions; module contracts and tests establish
algorithm-specific behavior under [CORE-06](../introduction.md#core-06).

The running program then creates ordinary values and calls ordinary modules.
A static specialization is a compiler object, not a value passed to an ordinary
input. A dictionary instance created from an exported dictionary Type is an
ordinary value; the factory that supplied its Type is not that instance.

**Design rationale.** Separate static modules make the phase visible in source.
Ordinary declarations retain their name/type shape. Their structural Type
expressions can apply named constructors and select one export under
[TPL-15](templates.md#tpl-15); standalone equations and public static results
remain in `@...`. Named static bindings identify the
intended parameter or result without depending on declaration order. The
compiler substitutes structural Type/Template expressions; it does not execute
arbitrary Ama or native code to discover types.

The examples below use descriptive module names and assume the stated ordinary
constructors exist. Their names are not additional standard-library facilities.

## The static module

<a id="tpl-12"></a>

**TPL-12.** A static module begins with `@...` and contains a graph of static
result definitions and named uses of other static modules. It has no `@=`, `@<`,
`@>`, `@!` or `@C` section. Blank lines and comments follow the ordinary
source-layout rules. An empty graph has no inputs or exports.
It declares a static type marker under [TPL-14](#tpl-14). A nonempty graph is
a static producer; it has named outputs and no ordinary application Type of
its own. Classify these cases from the complete valid source, before inspecting
uses. Invalid graph statements do not make a file an empty marker.

Its values are Type and Template metadata under
[TPL-01/TPL-07/TPL-08](templates.md#tpl-01). A Type denotes one ordinary module
type or a static type marker. A Template retains an actual original
constructor, its fixed captures and its still-unfilled named formals. A static producer's
own specialization records its inputs and outputs for compilation; it does not
provide an additional ordinary application Type or an executable instance.

The graph has no ordinary variables, mutable result cells, references, lists of
runtime values, defaults, literals, ordinary initializers, acquisition, reset,
copy or continuation state. There is no runtime control flow, module execution,
native hook or refusal handler in this graph. Static names use the complete
`...name` spelling under TPL-01; names and substitutions are not source-text
replacement.

Only static modules contain standalone static result pairs, producer-use graph
statements and named static output bindings. Ordinary declaration type
expressions may contain named static input bindings and select one export under
[TPL-15](templates.md#tpl-15) and the source form
[below](#using-an-exported-type-in-ordinary-code). An ordinary template path
retains field inference for omitted formals under
[TPL-04](templates.md#tpl-04). These declarations do not create standalone static
graphs or turn the enclosing module into a static producer.

## Static type markers

**Explanation.** Some type arguments describe a purpose rather than stored data:
node versus edge identifiers, a coordinate system, or a stage of processing.
Such a name needs an exact identity but no instance. A marker file supplies that
identity without an artificial variable, empty reference or executable body.
Using the existing static annotation keeps its compile-time role visible.

<a id="tpl-14"></a>

**TPL-14.** A complete source file whose only non-layout construct is the single
zero-indent `@...` annotation declares one closed nominal **static type marker**.
Leading/trailing blank lines, valid zero-indent comment lines and a valid
trailing comment on the annotation follow
[SYN-02/SYN-06](source-and-names.md#syn-02) and do not change this
classification. A zero-byte file or a file containing only comments is a draft
under SYN-04, not a marker. Every source line must still satisfy the ordinary
UTF-8, spacing, indentation and annotation rules.

The marker's identity is its original full logical module identity, including
root kind and every path component, under SRC-01/SRC-02/SRC-05. Two marker files
with different logical identities denote different Types even when their bytes
are identical. Physical `name+/` storage, aliases and named exports do not add
another identity. Loading the same marker through several uses preserves one
identity; source conflicts follow the normal module-resolution rules.

A marker belongs to **Type** under TPL-07. It has no free inputs, residual
formals or named exports. Its concrete module path is a static value-expression
head, and may occur as a named static input's right-hand expression. A visible
alias can forward it in the same positions. Static results and Type-valued
static inputs may carry markers; compare their exact identities in equations
and preserve them in complete specialization keys under TPL-05. Marker spelling
does not infer a category, select an implementation or match another marker.

A marker is not Template and cannot be applied with input bindings. It is not
a multioutput producer: selecting a named output from it fails, and a marker
path alone is not a producer-use statement. A nonempty static producer remains
subject to TPL-12 and cannot be used as its own Type. Adding a graph statement
to a marker file changes this classification; its uses must then satisfy the
producer rules. No implicit own-Type export or conversion preserves the old
marker meaning.

A marker has no runtime instances, fields, methods, constants, native payload,
busy/continuation state, lifecycle execution or runtime ABI. It requires no
runtime slot or value transfer. Reject it as the module Type of any ordinary
variable, reference target, owning-list element, reference-list target or shared
constant, even when the reference/list is empty or the declaration is never
executed. Apply this check after placeholder substitution and named static
export selection as well as for a direct marker path. A marker cannot be an
executable entry, native definition or compiled-library export root.

Markers are static identities carried by the existing Type/Template graph;
they do not add a runtime declaration category or an exception to TYPE-01.
Ordinary interface expressions introduce parameters under TPL-02/TPL-15, but a
marker substitution must still be rejected wherever an ordinary storable Type
is required. No runtime sample marker, implicit instance, numeric static
argument, static type-case dispatch or recursion exception is introduced. The
existing definition, substitution and creation checks remain required.

**Example: two complete marker files.** File `.marks.node` contains:

```text
@...
```

File `.marks.edge` contains the same source. Their full paths distinguish them.
Neither file defines a named result or creates an object.

**Example: complete `.forward kind`.**

```text
@...
...forwarded
...kind
```

This producer forwards its supplied Type or Template. A complete `.kinds`
assembly can pass a marker and expose several aliases:

```text
@...
.forward kind
 ...kind < .marks.node
 ...forwarded > ...node kind

...same node kind
.marks.node

...edge kind
.marks.edge
```

`...node kind` and `...same node kind` denote the same marker Type;
`...edge kind` denotes a different one. The aliases do not acquire `.kinds` as
a new nominal origin. This example uses only compile-time metadata; attempting
to declare an ordinary field with any of these marker Types is invalid.

## Defining and forwarding a result

A result definition has two same-level head lines: the complete public name,
then a static value expression. Named inputs of that expression are indented
one level deeper. This pair names the value of the expression itself; it does
not select a result member of the expression's head.

**Example: complete `.prepare types`.** Assume `.dictionary` is an ordinary
constructor with the two Type formals `...key type` and `...value type`:

```text
@...
...dictionary type
.dictionary
 ...key type < ..text
 ...value type < ..integer

...dictionaries for text
.dictionary
 ...key type < ..text

...integer type
..integer
```

There are three public results. `...dictionary type` is the concrete
`Dictionary(Text, Integer)` Type. `...dictionaries for text` is a Template with
its key fixed to text and its value formal still open. `...integer type` is the
supplied integer Type. None of these definitions creates a dictionary or integer
instance.

A result expression may be an ordinary constructor path, a marker path, a
supplied Type or a supplied Template. A concrete Type cannot be applied again
as a constructor. Applying named inputs to a Template uses its actual residual
formals under
TPL-08. Any omitted residual formals remain open: the expression returns a
Template until all its formals are filled. Supplying a partial Template is not
the same operation as leaving the identity of a producer input unknown.

**Example: complete forwarding module.**

```text
@...
...forwarded
...source
```

Here `...source` is an external static input. The output preserves that supplied
Type or Template, including a Template's original constructor and captures. The
two names do not create two nominal types. Their categories and actual metadata
are checked at specialization; descriptive words in their names do not determine
the category.

Constructor expressions can nest as input values. Each binding's children are
indented one further level; the outer pair still names the outer expression.

**Example: a finite nested expression.** Assume `.box` has the Type formal
`...element type`:

```text
@...
...twice boxed
.box
 ...element type < .box
  ...element type < ..integer
```

This is the finite expression `Box(Box(Integer))`. Merely writing nested
arguments does not make one box specialization require another application in
its own definition; creation provenance follows
[TPL-11](templates.md#tpl-11).

## Calling a static producer

A static producer use names a concrete path to a static module, followed by
zero or more named inputs and one or more named output selections. An input is
`...formal < expression`. An output is `...result > ...receiver`. The left name
belongs to the selected producer; source expressions and receiver names belong
to the supplying module.

**Example: complete `.selected types`, using `.prepare types` above.**

```text
@...
.prepare types
 ...dictionary type > ...dictionary
 ...dictionaries for text > ...family
 ...integer type > ...integer
```

The three receivers are public outputs of `.selected types`. Selecting one
producer result does not discard the other required definitions or change the
producer's identity. A call can select only the outputs it needs, but every
declared output of its required producer must still pass checking.

All static application bindings are named. Positional inputs, positional output
selection, skips, bare `<`, bare `>` and a bare `...` binding marker are not
source forms. Unknown or repeated foreign labels are errors, including equal
repeated bindings. A binding must contain its source or receiver. Named inputs
precede named outputs at each application level. Source nesting follows SYN-06;
there is no static ordinal counter or implicit first result.

A static producer has named outputs rather than a value denoting its own Type.
It cannot be the right-hand value of a result pair or an inline constructor
argument. Select its result into a name, then use that name as the value of
another expression. A producer use without an output selection is not a graph
statement. This introduces no output-free or effectful compile-time call.

The actual nonempty producer is determined by its explicit path. A marker path
denotes its Type in a value expression instead of supplying named outputs.
A supplied Template
input denotes an ordinary constructor under TPL-07, not an arbitrary function
that executes a static module or produces a bundle of named results. Passing
such static functions as values would require a separate language facility.

## Inputs, public names and scope

Before classifying source occurrences, collect all result-pair names and every
named output receiver in the entire static module. This includes occurrences
appearing after a use. Every collected name is public; using an output later as
an intermediate does not make it private or remove it from the interface.

After excluding these output names, every free local `...name` used as a source
or value-expression head is an external static input. Foreign input/output
labels, concrete path components and formals belonging to an actual constructor
are not local input introductions. Equal spellings in different modules do not
connect inputs; only explicit bindings connect their scoped identities.

The canonical collection order of external inputs is their first eligible local
source/head occurrence after this whole-module output classification. Keep that
order for deterministic original formal identities and diagnostics. It creates
no positional static binding interface; callers supply these inputs by name.

All collected input and output names are visible throughout their declaring
static module. A later definition can supply an earlier expression. This is a
dependency graph, not a sequence of runtime assignments. In particular, the
compiler does not turn a forward reference to an output into an extra external
input.

**Example: complete forward reference.**

```text
@...
...boxed integer
.box
 ...element type < ...integer type

...integer type
..integer
```

The graph has two outputs and no external input. The output `...integer type`
is known from its definition even though that pair appears later in source.

Result pairs declare their names exactly once. A repeated pair declaration is
an error. A named selection into a pair-declared name imposes exact equality
with that definition, including when the selection precedes the pair. If no
pair declares a receiver, its first receiver occurrence in lexical source order
defines its immutable alias; later receipts into that name impose equality.
This binder classification is fixed by source syntax before equation solving.
It never depends on the order in which compiler work happens to finish.

The output name cannot be reassigned, shadowed or reopened. Type equality is
full nominal identity. Template equality includes the actual constructor,
fixed captures and complete actual residual signature. A new name merely names
its defining expression; it supplies no independent evidence that could select
a concrete type or invent a producer input.

Moving independent equations does not change the solution. Moving which
occurrence defines a reused receiver can change its definition dependencies;
that is a source change, not a different execution schedule. Preserve the
distinction between a definition and a later equality check when checking
[cycles](#separate-graphs-and-cycle-checks).

## Requirements inferred from uses

A static input need not declare an exhaustive expected Template signature.
Check its uses under [TPL-07](templates.md#tpl-07) against the actual metadata
provided by its caller. Infer requirements from all uses, including uses after
partial application, rather than from the first textual use or first concrete
constructor supplied during compilation.

**Example: complete `.prepare dictionaries`.**

```text
@...
...dictionaries for key
...dictionary family
 ...key type < ...key type

...integer dictionary
...dictionary family
 ...key type < ...key type
 ...value type < ..integer
```

Its external inputs are `...dictionary family` and `...key type`. Both output
pairs are public. The named applications require suitable residual formals
`...key type` and `...value type` on the supplied family. They do not state that
these are its only formals.

If the supplied family has exactly those two Type formals, the first output
retains the value formal and the second is concrete Type. If it also has an
unfilled `...allocator type`, the first output retains value and allocator, and
the second still retains allocator. The compiler must not silently remove that
extra formal or choose a default. A caller can supply a family whose allocator
was already fixed, or a later static expression can fill it.

The output category and residual metadata therefore follow the actual supplied
constructor and captures. An ordinary declaration still requires Type, not an
unfinished Template. Categories, full captures and residual names remain exact
metadata even though use requirements need not enumerate an exhaustive input
signature. Passing a constructor through an opaque input does not make it an
ordinary value or allow runtime reflection.

## Inference and complete producers

When a static producer use omits an input, retain a distinct inference term for
that actual formal at that lexical application occurrence. Do not turn the
omission into a new free input of the supplying module. Its identity includes
the enclosing specialization, original application site and actual formal.

Substitute explicit and inferred inputs into the producer's structural output
definitions while preserving scope. Receiver equalities and other applicable
static equations can determine missing inputs under TPL-10. No definition is
executed as an ordinary module body. Before admitting any selected result,
every producer input must be uniquely determined, including inputs not reflected
in the selected outputs. A Template-valued input is determined when its actual
origin, fixed captures and complete residual metadata are known; its own
residual parameters need not have been filled.

**Example: complete `.type identity`.**

```text
@...
...same type
...type
```

An explicitly defined receiver can constrain the omitted producer input:

```text
@...
...known type
..integer

.type identity
 ...same type > ...known type
```

The identity producer's `...type` is integer. By contrast, selecting that result
only into a fresh `...unknown type` supplies no anchor: the input and result
remain undetermined. Repeating the fresh alias supplies no additional evidence.

**Explanation: information can be lost.** In mathematical notation, let
`Factory(K, V)` return only `List(V)`. An expected `List(Integer)` determines
`V`, but contains no information about `K`. Supply `K` or determine it through
another permitted equation. The compiler does not invent an inverse, enumerate
constructors, search for a module matching an output name or select an unused
input arbitrarily.

Full producer determination and a returned Template's residuals are separate
requirements. A producer with all its inputs known may return a family that
deliberately retains some of its own parameters. That is finite static metadata,
not an unresolved producer specialization.

## Using an exported Type in ordinary code

An ordinary declaration can select one named export of a static producer as
part of its type expression under [TPL-15](templates.md#tpl-15). Its head
contains the static module's concrete path. Zero or more named input bindings
may follow, then exactly one complete bare exported name at the same child
indentation. A closed assembly retains the shortest form:

```text
first
.prepare types
 ...dictionary type

second
.prepare types
 ...dictionary type
```

Both fields have the same dictionary Type and independent runtime storage. The
static module is not instantiated as an ordinary parameter or field. A producer
with open inputs can receive them explicitly. For example, assume `.assemble`
has inputs `...key` and `...value` and exports `...dictionary type`:

```text
items
.assemble
 ...key < ..text
 ...value < ..integer
 ...dictionary type
```

The selector is mandatory even when the producer exports only one name. It has
no ordinal form, suffix or children. Input bindings precede it; they belong to
the producer head and do not apply the selected export again. Comments and
blank lines follow the ordinary layout rules. Initializers, when permitted,
follow the completed type expression at their usual depth:

```text
number
.prepare types
 ...integer type
 |0
```

Recognize named bindings and the bare selector from their source shapes before
loading the path's file. Semantic resolution then requires that the head denote
a static producer and that its exact exported name exist. Under an ordinary
constructor or marker head, a bare export selector is an error; do not
reinterpret it as an input binding or initializer. It declares no
free placeholder, alias or ordinary variable in the using module.

The declaration may remain symbolic in an open ordinary template. Its named
inputs, selected-output equation and other permitted typed connections are
solved together under TPL-03/TPL-10. Omitted producer inputs retain distinct
application-local inference variables under
[inference and complete producers](#inference-and-complete-producers); they are
not additional owner parameters. Before the concrete outer specialization is
admitted, every required producer input must be fully determined, including
inputs absent from the selected result. A concrete expected result may supply
structural evidence, but cannot select an unknown producer, invent a discarded
input or authorize an arbitrary inverse for a noninjective output.

The final declaration expression must yield one complete Type under TPL-15.
Runtime storage requires an ordinary Type; a marker, a residual Template and
the producer itself cannot type that storage. A determined producer may export
other Template-valued results without invalidating the selected Type, but every
required output must pass checking.

A complete nominal identity can be referenced while its required definition
group is still being checked. Do not require every ordinary body in a finite
nominal cycle to have finished validation before that identity can be named.
Before executable admission, finish the producer, every required output and
every actual dependency check. This reference grants no early successful
validation, equal-pending-key assumption or skipped definition.

## Composition and specialization identity

A named producer result can feed a result expression or another producer's
named input. The Type/Template distinction applies at each use. A static
producer with multiple outputs never implicitly supplies its first output as
an argument value.

**Example: complete `.composed types`.** Supply `.prepare dictionaries` with
the two-formal dictionary constructor used above, then finish its returned
family in a new pair:

```text
@...
.prepare dictionaries
 ...dictionary family < .dictionary
 ...key type < ..text
 ...dictionaries for key > ...text dictionaries
 ...integer dictionary > ...text integer dictionary

...text text dictionary
...text dictionaries
 ...value type < ..text
```

The two receivers and the final pair are all exports. The incoming family and
key are fully determined producer inputs, while its first output deliberately
retains a value formal. The last pair binds that residual formal without
changing the other returned Type or the original family. An intermediate remains
public even when another graph node uses it. To forward a result unchanged,
define another result pair whose expression is its name.

Aliases do not manufacture nominal types. Two producers returning the same
ordinary constructor with the same complete bindings return the same Type.
The result retains its constructor's origin, not a new origin formed from a
factory or alias name. Equal partial constructors retain equal captures and
residual metadata. Different producers still have their own complete
specialization identities and required checks. Preserve every producer input
in that identity, even when generated runtime code does not use it.

Receiving or forwarding a Type creates no instance. Later ordinary instances
of that Type retain their independent identity, lifecycle and state. No
static factory instance, Type value or Template value crosses the native or
ordinary runtime boundary as hidden data.

## Separate graphs and cycle checks

Static equations and ordinary recursive structures use different graphs.

- **Definition expansion:** nodes identify result-pair or defining receiver
  expressions in their scoped producer/application environments. Add an edge
  when evaluating such a definition requires another static definition's value,
  including a selected foreign output. Reject directed definition cycles with
  their source locations. An equality check against an already defined receiver
  does not replace or redefine that receiver's expression. Equality cycles and
  TPL-10's occurs check remain separate.
- **Specialization creation:** retain the actual ordinary and static producers
  required by applications under TPL-11. Identical complete nominal keys can
  close a finite graph; a creation path to a different specialization of the
  same original template is forbidden. Finite written argument nesting is not
  itself an application originating in the argument type's own definition.
- **Instance construction and other obligations:** ordinary reference fields
  and initially empty lists can make runtime construction finite. Eager scalar
  ownership, constant initialization and runtime availability still follow their
  own rules. Closing a type graph does not waive them.

**Example: a forbidden definition cycle.**

```text
@...
...a
...b

...b
...a
```

Both names are collected outputs, so neither becomes an external input. Their
definitions depend on each other and are rejected. In contrast, an ordinary
`Node(Integer)` may refer to that same complete Type through an initially empty
reference under TPL-11/TYPE-04. Such a reference does not read a static output
back through its own defining expression.

Static producer uses retain their real dependency and creation obligations even
when the caller selects only one output. Applications in static definitions
belong to their lexical definition owner. Returning a supplied Type adds no
fictitious constructor application. A returned partial constructor remains
finite metadata until applied; later applications retain the actual original
constructor and captures. Shared aliases and warm cache entries must not hide
an application or an unequal-key path.

Before following a dependency, apply the exact closed-key/pending-key check in
TPL-11. Unknown repeated keys are not evidence of equality. Suspend that
expansion and use only genuine permitted refinements; do not instantiate an
unbounded chain to search for an answer. An unresolved required obligation after
refinements are exhausted is underdetermined. Check actual static-definition
cycles independently even when a nominal specialization key closes.

Ordinary declarations may contain a named projection as part of the structural
type expression of TPL-15. Its explicit origin, scoped equations and selected
result are checked under these same rules. Do not admit a pending recursive
specialization merely to preview its interface, assume an unresolved key equal,
or follow a cyclic static value definition. A concrete ordinary context supplies
equations, not a cycle-check or complete-producer exception. Extra compiler
passes do not change these obligations.

## Checking and implementation

Check every declared output of every required static producer, including
unselected outputs. Check all normal type, operation, lifecycle, shared-constant,
ownership and native-boundary obligations of the resulting ordinary types.
Static expressions cannot inspect runtime values, list lengths/contents,
initialization flags, branch results or native effects. Numeric static arguments,
value-indexed types and arbitrary compile-time Ama execution are outside this
Type/Template graph. Structural checking establishes exact concrete identities,
finite admissible composition and compatibility with ordinary operations; it
does not prove a comparator's ordering laws or another algorithm-specific
contract. Those properties belong to module and composition tests under
[CORE-06](../introduction.md#core-06).

An exact checker can proceed as follows:

1. Parse immutable syntax. Classify module kind from its marker. Collect pair
   names and receiver names globally, classify unique definitions versus exact
   equality receipts, then collect remaining free local source names as inputs.
2. Retain original application occurrences, scoped symbolic input environments
   and output equations. Resolve a foreign name only in its actual original
   constructor or producer, never by searching candidate modules.
3. Refine the structural equations under TPL-10 and use requirements under
   TPL-07. Track each partial application's actual residual metadata. Follow
   guarded definition/application dependencies under TPL-11, retaining pending
   key comparisons rather than speculative copies.
4. Revisit a blocked obligation only when actual new equations or metadata can
   refine it. An iteration without new information is not progress. Reject
   contradictions, definition cycles, forbidden creation paths and unresolved
   required inputs/origins/captures; use no inference budget or heuristic.
5. Finish complete producer and required ordinary-definition validation before
   publishing admissible exports or executable types. Preserve provenance on
   every cache hit and when equal closed keys merge. Lower only ordinary runtime
   operations and storage.

**Correctness rationale.** Scoped names make explicit bindings independent of
unrelated spellings and discovery order. Whole-graph collection prevents forward
outputs from becoming accidental inputs. Named lookup identifies the actual
formal/result; partial application retains every unfixed formal instead of
guessing an arity. Acyclic definitions have a dependency order. The separate
TPL-11 rule bounds required specialization creation without executing module
bodies. Exact structural solving and complete closure establish determined
identities, or report a conflict or missing evidence. These are separate checks:
cache reuse, aliasing and a concrete selected output cannot substitute for any
one of them.

## Examples of rejected programs

**Examples: fragments with the stated interfaces assumed.**

| Case | Required diagnosis |
|---|---|
| Put an ordinary variable, runtime initializer or `@!` body in `@...` | Invalid static-module source form |
| Leave a result-pair name without its expression | Incomplete static result definition |
| Give a result `&`, `[]`, a literal or ordinary initializer | Invalid static result source form |
| Declare the same result pair twice | Duplicate static result definition |
| Supply a positional binding, skip or bare capture marker | Invalid static binding source form |
| Supply input after a named output at the same level | Invalid static phase order |
| Repeat a foreign input or output label in one application | Duplicate named binding |
| Read an ordinary runtime field as a static output | Not a declared static producer result |
| Use a static producer as the value of a pair or argument | Static producer has no own Type/Template value |
| Rebind a Template's already captured formal | Nonresidual input binding |
| Ignore extra unfilled actual formals and use the result as Type | Incomplete Template where Type is required |
| Receive a different determined Type into an existing output name | Conflicting immutable equality |
| Receive identity output only into a fresh name with no input evidence | Underdetermined producer input |
| Leave a producer input absent from an otherwise concrete output unknown | Underdetermined producer input |
| Choose an origin by matching only an output name or desired Type | Missing independent producer evidence |
| Define outputs through a direct or mutual expansion cycle | Cyclic static value definition |
| Hide an invalid constructor application in an unselected output | Invalid required producer specialization |
| Omit the ordinary named selector or select by ordinal | Missing or invalid static export reference |
| Select a partial Template as an ordinary field Type | Concrete Type required |
| Leave a producer input unresolved after combining the declaration and permitted typed connections | Complete producer required |
| Use a declaration's expected Type to guess an unknown producer or an unused input | Missing structural evidence |
| Depend on runtime data, list length, native effects or initialization flags | Runtime-dependent static definition |

These source forms are requirements, not evidence of compiler support.
