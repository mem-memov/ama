# Declaring values, choosing types, and understanding scope

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Writing an explicit declaration](#writing-an-explicit-declaration)
- [Storage that survives a call](#storage-that-survives-a-call)
- [Where a name can be used](#where-a-name-can-be-used)
- [Declaring a field at its first receiver](#declaring-a-field-at-its-first-receiver)
- [Supplying literal data](#supplying-literal-data)
- [Sharing constants](#sharing-constants)

<!-- /chapter-contents -->

An Ama runtime name identifies storage. Its type is a module, and its suffix
says whether that storage owns a value, holds a reference, owns a list, or holds a list of
references. Keeping those two decisions separate makes ownership visible at the
place where a name is used.

**Explanation.** Most declarations create fields of the containing instance.
They are constructed when that instance is created, even if their written
declaration is inside a branch that never runs. Assignments change those fields
during execution. A newly introduced foreach reference is the one scoped
exception: it exists for that loop body.

Start with explicit declarations below, then read about storage lifetime and name
visibility. Receiver-inferred declarations are a shorter spelling with the same
persistent-storage behavior. [Values and references](values-and-references.md)
defines what transfers between these storage forms do.

## Writing an explicit declaration

### Names, types, and storage forms

<a id="type-01"></a>

**TYPE-01.** Every variable has a type defined by a module. An explicit declaration
answers three questions in order: what is the name and storage form, which module
defines its type, and what are its initial contents?

Write the name and type on separate lines at the same indentation. A type line
starts with a concrete module path or a scoped placeholder permitted by
[TPL-01/TPL-02](templates.md#tpl-01). Its static application block, when present,
is one space deeper and precedes ordinary initializer lines at that same depth;
[TPL-07/TPL-08](templates.md#tpl-07) define the type-expression forms, with
static inputs, projections and aliases governed by
[TPL-12](static-results.md#tpl-12). A path naming a template is specialized
statically under [TPL-03/TPL-04](templates.md#tpl-03), including when it types a nested field.
The storage marker directly adjoins the name; it never belongs to the type line
and never has an intervening space.

Static formals in `@<` and static results in `@>` use
[TPL-12](static-results.md#tpl-12)'s separate declaration forms. They describe
compile-time Type/Template parameters and expressions rather than any runtime
storage form below. Static application and result projection do not construct a
value or execute a module body. A static result's second line defines its value;
an ordinary declaration's second line specifies the type of its storage.

There are two ways to omit an explicit type line. A receiver can introduce a
persistent field whose type comes from its source or selected result (TYPE-06).
A new foreach reference binding gets its type from the source list and is local
to the loop body (FLOW-05). These are distinct declaration rules.

| Declaration name | Storage meaning |
|---|---|
| `item` | Owned instance/value |
| `target&` | Non-owning typed reference |
| `items[]` | Dynamic list owning its elements |
| `targets&[]` | Dynamic list containing non-owning references |

```text
item
.shop.item

target&
.shop.item
 &item

items[]
.shop.item

targets&[]
.shop.item
 &item
```

### Initial contents and construction order

<a id="type-02"></a>

**TYPE-02.** A scalar has at most one initializer. A list can have several, with each
initializer creating exactly one element. A whole list is not an element
initializer: initialization never implicitly splices one list into another or
creates nested lists. Every source must have a compatible type and already be
initialized.

Initialize declarations in their declaration order.
For persistent body fields, that order is lexical source order, traversing nested
branch/loop bodies at their written location; both branch alternatives contribute
fields unconditionally. A receiver-inferred declaration contributes its field at
the receiver's source location, with ordinary default initialization under
TYPE-06. Blocks never make construction conditional on execution.
The scoped foreach binding itself is not a persistent body declaration: its
runtime activation follows FLOW-05, while ordinary declarations in that loop's
body still follow this unconditional construction order.

### Data representations belong to modules

<a id="type-03"></a>

**TYPE-03.** There is no privileged language-level `size_t`, machine integer,
raw address, or universal truth value. Integers, bytes, text, and other data
representations belong to modules and their library contracts. Compiler-internal
indices, counters, sizes, and continuation selectors may use C integer types.

### Creating owned state without running it

<a id="type-04"></a>

**TYPE-04.** Owned state is initialized according to its type, including recursively
owned children, without executing their module bodies. Lists start empty unless
initialized. Infinite eager ownership cycles are invalid; a reference or empty
dynamic list does not itself require recursively allocating its target type.
Unsupported default construction must be diagnosed or fail through the defined
lifecycle protocol.

For eager construction, form a type edge for every owned scalar field and for
every owning-list field with one or more element initializers. Reference fields,
reference-list fields and initially empty owning lists add no eager edge. Reject
a cycle in this eager-construction graph: every such edge requires at least one
child construction before its owner can finish. Check initializer value
dependencies separately; absence of an eager cycle does not authorize reading
an uninitialized source. Module loading and clone-capability graphs have different
edges and must not be substituted for this graph.

### Exact compatibility between storage forms

<a id="type-05"></a>

**TYPE-05.** Type compatibility is nominal: the same complete module identity is
required, including all specialization bindings under
[TPL-05](templates.md#tpl-05), together with the storage-form rules below.
Matching layouts, similar names or a C cast do not make different Ama types
compatible. There is no implicit reference formation, dereference, scalar/list
conversion, list splicing or subtype conversion.

| Destination storage | Accepted ordinary source of the same module type | Effect |
|---|---|---|
| Owned scalar | Owned scalar access or scalar constant | Independent owned copy |
| Reference scalar | Reference access, prefix `&` of an owned scalar access, or standalone `&` of the containing module type | Copy/form a non-owning handle |
| Owning list | Whole owning list or owning constant list | Independent whole-list copy |
| Reference list | Whole reference list | Copy handle entries, without cloning targets |

An indexed owned element is scalar storage; an indexed reference-list entry is
reference storage. An ordinary reference source such as `target&` cannot implicitly
clone its referent into owned storage. Explicit `% target&` instead requests the
continuation clone in CONT-04. `%` of an owned or reference scalar may construct
or replace distinct owned scalar storage only; it does not produce a reference
or whole list. Clone capability and overlap rules still apply.

Each list initializer creates one element: apply the scalar rule for that list's
ownership form. A whole-list source is not one element. Declaration literals and
CALL-07 input literals construct owned scalars using the destination's `|`
capability; they cannot initialize reference storage. These rules also govern
parameter binding and result transfer. An indexed access in target position
selects an instance to execute/read; that is distinct from transferring a handle.

## Storage that survives a call

### Permanent body fields

<a id="state-01"></a>

**STATE-01.** Body declarations describe permanent instance state, including
ones inferred at assignment/output receivers and ones written inside branches
or loops.
Their initialization runs once at instance creation, not whenever execution
reaches the declaration's source location.
Assignments are executable operations and may update that state on every call.
FLOW-05's newly introduced foreach reference bindings are scoped runtime
bindings, not permanent body fields; their saved state exists only for an active
or suspended enclosing loop.

### Parameters and retained results

<a id="state-02"></a>

**STATE-02.** Parameters and results are interface storage. An entry-start call
reinitializes them for the invocation before running the body. Install parameters
in declaration order, choosing each supplied source/literal or its declared
default, then initialize results in declaration order under CALL-08. A later
default observes the already installed earlier fields, including supplied inputs.
Do not default-construct a supplied parameter and then overwrite it. Their final
values are retained after completion or suspension. Before the first call,
results have their declared initial values. A retained-result read exposes only
results, not parameters or private body fields.

### Shared immutable storage

<a id="state-03"></a>

**STATE-03.** `@=` declares one shared immutable part of a module, usable alongside
parameters/results and either Ama or native implementation. Its values belong to
the module identity, not to any executable instance. They are initialized once per execution context before
its root creation/execution under CONST-01 through CONST-06, never once per instance
or invocation. Earlier constants in the same module may be used by name in later
constant/interface/body initializers.

Shared constants are excluded from instance layouts, interface reset, ownership
trees, ordinary copying, and `%` continuation copying. Creating, copying, or
resuming an instance cannot duplicate or mutate them. They have no busy state,
slot generation, caller link, or execution position. They cannot be assignment
receivers or sources of mutable references. Shared storage is not an instance
pool or a callable singleton.

A constants-only module provides the shared part without executable instances.
It requires no pool and cannot run as the root. A static type namespace followed
by `>>` reads shared constants under CONST-03, never an instance's retained results.

Separate parallel execution contexts have separate constant values and native
value/resource lifecycles. They may share compiled code and immutable literal
bytes, but they must not expose one context's Ama constant storage or handles to
another. See [Asynchrony and parallelism](async-and-parallelism.md).


## Where a name can be used

**Explanation.** Names belong to the module in which they are declared. Calling another module does not make its private names visible to the caller.

### One base-name namespace per module

<a id="name-04"></a>

**NAME-04.** Each module has one declaration namespace covering shared constants,
parameters, results, and persistent body declarations, including declarations
inside branches/loops and receiver-inferred declarations under TYPE-06.
A complete base name may be declared only once in that module. `item`, `item&`,
`item[]`, and `item&[]` share the base name `item` and cannot coexist as separate
declarations. Storage suffixes express kind, not separate name namespaces.
Nested blocks do not introduce shadowing. FLOW-05 adds one exception to the
module-wide storage rule: a new foreach reference binding is local to its body.
It reuses a compatible visible variable rather than shadowing it. Disjoint loop
bodies may introduce independent bindings with the same base name. Such a
binding's generated identity includes its module and lexical foreach location,
so repeated local names cannot collide in generated storage or diagnostics.
Different modules may reuse base names.
Multiword names are indivisible namespace entries; their individual words do not
reserve names. Internal spaces are preserved in identity and generated-name hash
inputs, while dot delimiters and adjoining storage/access markers are excluded
from component/base-name identity. The root kind remains a separate part of
every qualified module identity under SRC-01 and NAME-01.

Resolve uses within this module namespace, require matching storage form, and
retain declaration-order initialization. Name visibility does not authorize use
of an uninitialized source or cyclic initialization. Duplicate-name diagnostics
identify both declarations and their storage forms. No suffix may silently select
a different declaration when a use is missing or mistyping its storage marker.

### Lexical visibility and no shadowing

<a id="name-06"></a>

**NAME-06.** Variable scope is lexical: determine it from the parsed source,
declaration classification, and enclosing foreach bodies, never from the runtime
call stack, branch outcome, current list contents, or a successful name/type
guess. Apply TYPE-06's receiver classification and FLOW-05's binding traversal
before resolving ordinary uses. Visibility does not change those rules: a later
explicit declaration still duplicates an earlier inferred field, even though
explicit declarations are collected for ordinary use and foreach resolution.

| Declaration kind | Where its name is visible |
|---|---|
| Explicit shared constant, parameter, result, or persistent body field | Throughout its declaring module, including text before its declaration and all nested executable bodies. Declaring a field inside a branch, loop, or `:` does not restrict its scope to that body. |
| TYPE-06 receiver-inferred persistent field | At its introducing receiver and subsequent source positions in that module, including outside the branch/loop/handler containing that receiver. An earlier unresolved use prevents later inference; it cannot be repaired by treating the later receiver as a forward declaration. |
| New FLOW-05 foreach-local reference | Executable uses in that foreach body and its nested bodies only. It is absent from its own collection-source expression, following code, sibling bodies, and instance-construction initializers. An enclosing binding remains available when resolving a nested loop's source. |

Static parameter, result and alias names belong to the separate static
namespace of [TPL-02](templates.md#tpl-02) and
[TPL-12](static-results.md#tpl-12), not this runtime field table. In particular,
collecting runtime declarations does not turn a later static result alias into
a forward-visible name.

#### Blocks and reuse

Ordinary `?`, `:`, and condition-loop bodies create no variable scope. A foreach
body scopes only a reference newly introduced by its header; other explicit or
inferred declarations written inside it remain module fields. A compatible
existing variable reused by a foreach keeps its original scope and lifetime.
Scope and lookup remain static when a body is skipped or a list is empty.

Shadowing is forbidden at every nesting depth. Repeating a module-field
declaration is a duplicate under NAME-04. At a foreach header, an existing
visible name must denote a compatible mutable scalar reference and is reused;
an incompatible name is an error, not permission to hide it with a new local.
This includes enclosing foreach-local references. Nested reuse updates the same
cell and does not restore its old value when the inner loop ends. Disjoint
foreach bodies may introduce independent locals with the same base name; a
later inferred module field outside an earlier local's scope may also use that
name without changing the earlier body's binding. All suffix forms share the
same base-name lookup, so changing `&` or `[]` cannot bypass these restrictions.

#### Visibility does not imply initialized or accessible storage

Visibility is distinct from initialization, storage duration, and access rights.
Reading an initializer source still requires that source to be initialized in
TYPE-02/LIFE-01 order; a visible later declaration does not authorize a forward
or cyclic initialization dependency. Shared `@=` initialization can use only
CONST-02's permitted sources, not instance fields. Persistent fields are
constructed once regardless of whether execution reaches their declaration;
an inferred receiver's executable transfer happens later. A foreach-local cell
is activated only when FLOW-05 selects an entry for a nonempty body, and is
unavailable to creation-time initializers of persistent fields. A handle copied
out of that cell retains its normal non-owning lifetime after the local name
leaves scope; neither the name nor ownership is exported by that copy.

#### A call does not change the caller’s name lookup

Unqualified executable uses select declarations of the containing module's
current instance, plus its active foreach locals and shared constants. Different
instances have independent instance fields; different modules may reuse names.
There is no implicit access to the private fields or parameters of a caller,
callee, or owning parent, and no dynamic caller-chain lookup. A call's input
labels select its target's `@<` declarations; instance result labels and
direct-result selectors select the appropriate target's `@>` declarations.
Shared-constant read labels and qualified constant access use the named module's
`@=` section; list-service members and their parameter/result labels use
LIST-04's intrinsic typed interfaces.
These interface lookups do not import that target's variable namespace into the
caller.

Under CALL-09/FLOW-04, ordinary input sources, indices, output receivers, and
refusal-handler bodies retain the initiating caller's lexical environment,
including visible foreach locals; descendant result selectors use only their
immediate parent's declared result interface. Declared defaults instead use
their declaring instance's context under CALL-08. The standalone self-reference
`&` is not a variable name and follows REF-01's containing-instance rule.

#### Example: a declaration inside a branch remains a field

**Example.** Assume `..bit` maps literal `0` to a false condition and `..integer`
supports the shown construction and copying:

```text
@!
ready
..bit
 |0
fallback
..integer
 |2

ready
?
 cached
 ..integer
  |7

cached
 fallback
```

`cached` is constructed with `7` when this instance is created. The false branch
does not suppress its construction or hide its name. The final assignment is
outside the branch and replaces its value with `2`. Redeclaring `cached` in an
inner block would be a duplicate rather than a second local variable.


## Declaring a field at its first receiver

**Explanation.** An assignment source or declared result supplies a scoped type
expression. Ama lets a receiver's first textual occurrence use that information
instead of repeating a type line. The expression may be concrete already or
become concrete through other typed connections before layout. The suffix stays
explicit so that ownership remains visible.

<a id="type-06"></a>

**TYPE-06.** A simple assignment or output receiver may declare a new persistent
field without a separate module-type line. This applies to standalone assignments,
`>` bindings on entry-start/resume calls, and `>>` bindings on retained-result
reads, including typed outputs of intrinsic list methods, and on shared-constant
reads. Existing named/positional and operation-mode rules still apply;
LIST-04's service parent has no ordinary result-copy bindings or resume. The
receiving name must be one of `name`,
`name&`, `name[]`, or `name&[]`; an indexed receiver cannot declare its collection
or index. Empty output placeholders declare nothing. Outside FLOW-05's scoped
foreach binding, no other operand position implicitly declares storage.
An assignment still requires exactly one source
line one indentation level below its receiver; omitting the type line does not
permit omitting this indentation or using a literal as an assignment source.

### The exact declaration-and-type algorithm

Classification depends on source order, not on the branches actually executed or
the order in which modules happen to load. Use this algorithm:

1. Parse the module structurally, without using declared-name lookup to choose
   its grammar. Collect all explicit runtime declarations, bind their scoped type
   expressions and static parameter names under TPL-02, and retain their storage
   forms. A type expression may still contain unresolved specialization variables;
   do not require a complete concrete type at this stage. This inventory is
   available throughout the module for ordinary lookup and for FLOW-05's
   foreach-header lookup. Static constructor formals are not runtime declarations.
   Collecting a later explicit declaration does not count as encountering its name at an
   earlier source position and does not suppress an earlier receiver-inferred
   declaration. If both declare that base name, report a duplicate.
2. Traverse the parsed module once in increasing source-byte order, visiting each
   branch, loop, and `:` body at its written location. Maintain the set of
   encountered module-variable base names, the scoped type terms and storage
   forms of earlier inferred fields, and the active lexical foreach bindings.
   All four storage spellings share a base name. A possibly multiword name is
   one occurrence, never a set
   of occurrences of its individual words.
3. At a foreach header, resolve the source list's declaration identity, storage
   form and element type term from the explicit inventory, earlier inferred
   fields and enclosing lexical bindings. Apply FLOW-05's name rule now: reuse
   the visible mutable scalar reference, if present, or introduce the specified
   local reference for that body. An existing name with another storage role is
   an error, not a reason to shadow it. Record type compatibility with the source
   element as an equation if its complete type is not known yet. Delaying that
   check never delays or changes the reuse/new-binding decision.
   New local-binding declarations and occurrences resolved to those locals do
   not enter the module-variable encountered set. An assignment/output receiver
   resolved to such a local updates that cell and cannot declare a module field.
4. For every other module-variable occurrence, inspect the encountered set before
   adding its base name. A simple assignment or output receiver whose base name
   is absent declares an inferred field. Other occurrences are ordinary uses or
   explicit declarations according to their syntax. Add each encountered base
   name, including unresolved uses. An earlier unresolved use therefore prevents
   inference at a later receiver.
5. Give each new inferred field a scoped type term immediately and connect it to
   its source or selected result; the term need not yet be concrete. An assignment's
   following source line supplies its transfer form through the table below.
   Resolve names in that line using explicit declarations, earlier inferred fields
   and visible foreach bindings; also visit their occurrences in source order.
   A dependency on the field currently being inferred is an error. For an output
   receiver, bind the operation target and selected interface position or label,
   retaining constraints for interface resolution if the target's specialization
   is still unresolved. Equate the new field's type to exactly that result's
   scoped type term. Constant reads first select a concrete namespace under
   CONST-03, then its existing `@=` declaration; list methods select LIST-04's
   fixed typed interface. A constant receiver cannot infer the namespace it
   reads from.
   Neither source execution nor runtime values supplies a type. Result selection
   uses the declared interface, not a search for suitable fields in a callee
   body; that body's ordinary static equations may refine the interface's terms.
6. Record exact compatibility of the receiver's written storage form with the
   source transfer form or selected result. Validate known facts now and retain
   checks requiring resolved types. Missing or mismatching `&`/`[]` is an error;
   do not insert a suffix, dereference, form a reference, or convert between
   scalar and list. Add the field identity, storage form and type term to the
   earlier-inferred inventory before visiting subsequent constructs. A valid
   later explicit declaration can resolve ordinary uses under the existing
   visibility rule; a later inferred receiver cannot. An invalid first occurrence
   never triggers a search for a different declaration site or classification.
7. Solve the type constraints together under TPL-03/TPL-10. Typed inputs, already
   declared output receivers, typed defaults, declaration initializers,
   assignments and reference/alias connections may refine an earlier field's
   type term. An explicitly typed generic destination whose arguments are still
   unknown contributes equations, not a concrete anchor. A new inferred receiver
   likewise supplies no independent concrete type. Reject inconsistent or
   unresolved required types and infinite substitution terms; then validate all
   namespaces, interfaces, initialization, ownership, capabilities and transfer
   checks before layout or execution. Type refinement never repairs an unresolved
   earlier name use or changes a lexical binding established by steps 1 through 6.

Module-path components, static parameter/signature names and binding labels,
qualified constant member names, callee-side binding
labels, direct-result selector base names, comments, and literal payloads are not
local variable occurrences. They do not enter the encountered set. Names inside
an index and a local constant used by its unqualified name do enter it. At an
indexed receiver, visit its collection and index uses; the receiver cannot declare
either name. Classify header/source occurrences before entering a foreach body's
new local scope. These rules keep the traversal lexical and unambiguous.

### Deriving the storage form

Assignment-source inference uses the following exact rules. An indexed owned
element or reference-list entry is scalar; a whole-list source retains its list
form. The scoped module type comes from the source's declared type expression,
never its current runtime contents. It may remain symbolic until the static
constraints are solved.

| Assignment source | Inferred module type term | Required receiver storage form |
|---|---|---|
| Owned scalar access or scalar constant | Source module type | Owned scalar |
| Reference access, including an indexed reference-list entry | Referent module type | Reference scalar |
| `&item` or `&items[index]` | Selected owned instance's module type | Reference scalar |
| Standalone `&` | Containing module's complete type expression | Reference scalar |
| Whole owning list or owning constant list | Element module type | Owning list |
| Whole reference list | Referent element module type | Reference list |
| `% owned` or `% target&`, including indexed scalar operands | Source/referent module type | Owned scalar |

The last row is an explicit continuation copy under CONT-04; all other rows use
ordinary TYPE-05 transfer semantics. No storage suffix is inserted or inferred
away, and no implicit dereference, reference formation, list conversion, or
literal-type inference is introduced.

### Why the algorithm has one answer

After any scanned prefix, the encountered-name set is exactly the set of
module-variable base names occurring in that prefix. A visited module-variable
assignment/output receiver declares storage if and only if it is simple and
its base name is absent immediately before that occurrence. Resolving lexical bindings at their headers under FLOW-05 makes
classification unique. A validation failure rejects the program; it never
changes the classification or starts another inference attempt. It does not change existing visibility rules for
explicit declarations or permit forward/cyclic initializer dependencies.

The declaration identity of an operation target or assignment source comes from
an explicit declaration, an earlier receiver-inferred declaration or a visible
foreach binding. Indexed operands obey the same name rule. References to other
inferred declarations therefore decrease their introducing source positions;
valid explicit declarations may occur later under the existing visibility rule.
This establishes lexical bindings without needing every type to be concrete.
A CALL-09 selector then uses its enclosing operation's declared result interface,
retaining the necessary type/interface constraint if that operation is not yet
specialized. It never searches a body or caller namespace for a matching member.

Type equations are a separate graph and need not decrease source positions.
For example, a later typed assignment can constrain the type term of a field
already introduced by an earlier result receiver. Equality cycles between type
variables are not errors by themselves: a concrete anchor may determine their
common type. Without that anchor they remain underdetermined. An infinite
substitution term still fails the occurs check, and an inferred field cannot
provide its own assignment-source type at its introducing receiver. TPL-03 and
TPL-10 define exact propagation and uniqueness, without guessing from literal
bytes, capabilities or runtime execution. Name classification never reruns after
type solving, even when a constraint fails. Cross-module loading, initialization
order and eager ownership construction remain independent checks; a solved type
does not authorize a forward or cyclic value initializer.

### When the inferred field is constructed

The inferred field belongs to the containing module's namespace, not to a callee
or a branch-local scope. It is constructed once with the enclosing instance, in
body-field declaration order: an owned scalar uses its type's default
construction, and references/lists start empty. The assignment or output binding
is an executable transfer into that already live field, not its creation-time
initializer. A later declaration initializer can observe only that field's
creation-time value, never a future assignment source or call result. Repeated
assignments, calls, loop iterations, and later receiver occurrences update the
same field. An output operation's acquisition refusal leaves it unchanged,
including its default/empty value before the first successful transfer.
Assignment failures use the ordinary REF-03/COPY-02 guarantees. Inferred owned
fields participate in TYPE-04's eager-construction graph and all normal
lifecycle/copy-capability checks; inference cannot bypass them.

### Examples of inferred fields

For example, assume `worker` has been declared and its results are, in order,
an owned scalar, a reference scalar, an owning list, and a reference list:

```text
worker !
 > value
 > target&
 > values[]
 > targets&[]
```

If these four base names have no earlier local occurrences, the receivers
declare four fields with the corresponding result types. A later `> value`
uses the existing field. In `result > result`, the left `result` is a callee
label and does not prevent the right `result` from declaring a local field.

An assignment may likewise introduce `new_var` by copying an existing value:

```text
is_ok
..bit
 |0

new_var
 is_ok
```

If `new_var` has no earlier local occurrence, it declares an owned `..bit` field.
The assignment copies the current value of `is_ok` each time execution reaches
it. The same rule permits `new_ref&` from `&is_ok`, a reference-list field from a
whole reference list, or an owned field from `% source`; their explicit suffixes
and ordinary transfer checks must match. A later assignment to `new_var` reuses
that field. `new_var` followed by an unindented `is_ok` is not an assignment.

**Example: a later typed connection resolves an earlier field's type.** Assume
the hypothetical `.producer` has one owned result whose Type parameter is
introduced in `@>`, no typed inputs and no other determining constraint. Assume
ordinary integer default construction and copying are available. This is a
complete caller module:

```text
@!
producer
.producer

producer !
 > received

known
..integer
 |7

received
 known
```

The first output binding introduces `received` and equates its type term to the
producer's result term; neither term is concrete yet. The later assignment uses
that existing field. Its source's declared `..integer` type supplies the concrete
anchor, so both terms resolve to integer before any instance is constructed.
The literal bytes `7` do not choose that type, and the assignment still executes
after the call. This does not permit a source name to refer forward to a later
inferred declaration: lexical name resolution remains the earlier pass.

## Supplying literal data

### The bytes after a vertical bar

<a id="lit-01"></a>

**LIT-01.** A literal begins with `|` in a declaration initializer or immediately
after the single space separating an entry-call input marker `<` from its operand.
Everything after that character to the physical line ending is literal payload.
`|` alone is empty; `||text` starts with a literal vertical bar. There are no
quotes, escapes, multiline literals, or interpolation rules.
Repeated and trailing payload spaces and `#` remain literal bytes; TAB is forbidden
even in payload under SYN-01. Do not insert, remove, or collapse a payload space.

```text
count
..integer
 |255

lines[]
..text
 |First line
 |
 |Quotes "and" # characters remain data
```

### Literal interchange format

<a id="lit-02"></a>

**LIT-02.** The declared module interprets literal payload. The literal interchange
contract is immutable UTF-8 bytes with an explicit length, excluding the marker
and physical newline. It must preserve embedded payload spaces and must not
require NUL termination or host-dependent `wchar_t`.

### A literal gets its interpretation from its destination

<a id="lit-03"></a>

**LIT-03.** Numeric literals are not a separate expression category. Literal
payload is permitted in declaration initialization and explicit entry-call inputs
under CALL-07. Assignments, conditions and indices still use named sources. Bare
`255` is not an argument expression: `< |255` supplies uninterpreted bytes to the
parameter type. The compiler MUST NOT introduce hidden numeric variables or infer
a numeric type from the payload.

### Validation and construction failures

<a id="lit-04"></a>

**LIT-04.** Each native-backed literal type must define accepted bytes, binary
representation, initialization errors, ownership, and cleanup. Literal bytes
cannot forge a checked instance reference. The `|` constructor receives an
immutable byte view and reports status when construction is attempted. Invalid
literal construction is a technical failure with declaration or call/binding
context; a compiler may diagnose it earlier only through a documented, equivalent validation mechanism,
not by executing arbitrary native constructors during compilation. C byte-view
typedef spelling is an implementation task, not a different literal semantics.

## Sharing constants

### What may be constant

<a id="const-01"></a>

**CONST-01.** Constants are immutable, independently copyable values, including
library numbers, text, and enumeration values. They cannot carry mutable-object
references or identities of open external resources such as files, sockets, or
devices. Reference/reference-list declarations are not permitted in `@=`.
Native storage must not hide prohibited links/resources. Constant-capable types
must support independent copying and construction without executing module bodies
or observing/changing mutable external state. Native constructors for constants
must not perform external I/O; internal allocation of value storage is permitted.
These are native/library contract obligations, not a new ABI capability symbol.

Reading or using a constant as a value source copies its value into compatible
receiver storage; it never grants mutable access to shared storage. Constants
cannot be execution/resume targets or reference-acquisition operands. Copying an
ordinary instance excludes the module's shared section as before; explicitly
copying a constant's value is a different, permitted operation.

### Construction and cleanup order

<a id="const-02"></a>

**CONST-02.** Initialize constants once in each execution context before that context’s root initialization, following
declaration order and initialized dependencies. The compiler checks visible
construction/initializer dependencies and rejects cycles or use-before-initialization.
Declaration initializers may use literals, previously initialized local constant
sources, or qualified constant sources under CONST-05. Include cross-module
constant dependencies in initialization planning; preserve declaration order
within each concrete module specialization and reject cycles. Qualifying a local
forward reference does not bypass that ordering. Required constant dependencies
initialize first, before the root can start.
Native construction must not introduce hidden initialization dependencies.
Do not execute an Ama module body or arbitrary native constructors at compiler
time. Build-time validation is allowed only through an explicitly equivalent,
documented validation mechanism. A runtime initialization failure identifies the
module/declaration and prevents root startup; clean completed constants and partial
construction through the lifecycle protocol. Dispose successful shared storage
once after executable instances, in reverse initialization order.

#### Exact initialization order

Plan initialization over individual constant declarations of concrete module
identities, not over a graph that merely says one module mentions another's type.
Different specializations have distinct nodes; equivalent specializations share
the same nodes under [TPL-05](templates.md#tpl-05). Add an ordering edge from each
constant to the preceding constant in its own concrete module, and a dependency
edge to each constant value its construction reads, including declared owned-field
initializers. Reject cycles in this graph. Choose the next dependency-ready
declaration deterministically by the bytewise full logical module path, then by
the bytewise canonical complete specialization-identity encoding when paths
are equal, then declaration ordinal. Use the versioned, unambiguous identity
encoding under [NAME-01/NAME-02](../implementation/compiler.md#name-01), including
all normalized static bindings in TPL-05's canonical order. Compare the identity
bytes themselves, not their hash, generated C identifier or an instance address.
A module declaring constants of its own executable value type is not thereby
cyclic: shared `@=` storage is not an owned field of that type. Eager owned-state
construction cycles are checked independently under TYPE-04.

### Reading another module’s constants

<a id="const-03"></a>

**CONST-03.** Read shared constants with a static type-expression namespace and
ordinary saved-read bindings:

```text
..palette
 red >> selected color
```

The namespace head is a concrete module path or a locally declared static
placeholder. It may have a static application block under
[TPL-07/TPL-08](templates.md#tpl-07) and
[TPL-12](static-results.md#tpl-12). Those bindings and static output projections
precede the read bindings at the same one-level-deeper indentation. Static `<`
bindings apply types or constructors; static `>` bindings project defined types
or constructors. Neither is an ordinary runtime input/result binding. The read requires
at least one `>>` with a receiver and a consistent named/positional read mode.
Ordinary runtime input `<`, runtime result transfer `>`, invocation `!`, resumption `%`, refusal
handlers and executable nested actions are not permitted in this namespace read.

Before selecting any member, resolve the namespace expression to one fully
concrete Type. Explicit static bindings and already determined local parameter
bindings may supply that Type; a known Template parameter may be applied to its
residual formals. A Type parameter cannot receive further static input bindings;
TPL-12 governs output projection from its known nominal origin.
An expression that remains open is an error. Without a static output phase,
this position introduces no fresh inference variables. With an explicit output
phase, [TPL-12](static-results.md#tpl-12) supplies per-expression terms for
omitted producer inputs; output equalities may determine them, and every
producer input must close before constant-member lookup. This never chooses a
default specialization. Do not infer namespace bindings from requested constant names,
positional ordinals, runtime receiver types or literal bytes. Independently
available type constraints, including TPL-12 static output equalities, may first
determine a local parameter; namespace constant-member lookup
cannot supply the evidence needed to choose its own owner. There is no search
for a module that happens to provide a requested constant.

Specialization-creation provenance follows [TPL-11](templates.md#tpl-11).
Selecting an ordinary nongeneric namespace without static outputs reuses its
fixed Type, just as a direct qualified source does under CONST-05. The namespace
is still independently loaded and checked; its constants retain their value
dependencies. Template applications and explicit static output phases keep
their required producer/application checks. Namespace selection never means
acquiring a default instance.

Resolve named requests only in the selected concrete type's own `@=` declarations;
positional requests follow their declaration order. Substitution does not import
or inherit constants of any argument type. A namespace cannot expose parameters,
body fields or instance results. Missing modules or members, draft sources,
incompatible types and mutable receivers in shared storage are compile-time
errors. Runtime value-copy failure remains technical. This operation does not
execute a body, acquire an instance, evaluate a getter or create a mutable alias.
An instance target with `>>` continues to expose only its retained `@>` results,
not constants. Local constants remain available directly by their declared names.

**Example: fragments using a selected specialization.** Assume `.container` is
the template defined in CONST-06 and `..integer` is independently copyable:

```text
.container
 ...element type < ..integer
 empty list[] >> working values[]
```

The static binding chooses the concrete namespace. The read copies its empty
constant into an owning list under the ordinary receiver-declaration rules.
Appending to `working values[]` later cannot mutate the shared empty list.
Omitting the static binding here leaves `.container` open and is an error;
`working values[]` does not choose the missing argument.

**Example: complete `.initial value`.** Assume, for this example only,
`..integer` declares a constant named `zero` of its own type. This is a library
assumption, not a required spelling or an added integer capability:

```text
@<
sample&
...value type

@>
result
...value type

@!
...value type
 zero >> result
```

An ordinary call supplying an integer reference determines `...value type` as
`..integer`; the body reads that concrete type's `zero` constant. No live target
is needed to determine the reference's declared module type. Passing a type with
no `zero` declaration is a compile-time error. A constant named `zero` whose
declared type differs from `...value type` also fails the result transfer check.

**Example: range bounds, body fragment.** Assume `...value type` is an already
determined local Type parameter whose actual type declares compatible constants
named `minimum` and `maximum`:

```text
...value type
 minimum >> lower bound
 maximum >> upper bound
```

The read obtains copies of that type's bounds. It does not infer the parameter
from those names, require all types to have bounds, or prove a mathematical
ordering law. Such meanings belong to the actual library's contract. If another
parameter's type also declares `minimum`, explicitly selecting its namespace
keeps the two owners separate without a name conflict.

**Example: body fragment using a supplied constructor.** Assume the interface
already declares `...container family` as a Template with exactly one residual
Type formal named `...element type`, and `...local type` is already determined:

```text
...container family
 ...element type < ...local type
 empty list[] >> working values[]
```

The constructor's signature does not promise an `empty list[]` constant. Check
that member and its type against the fully selected specialization, as for a
concrete module path. The head remains a static expression, never a runtime
variable or an invocation of the constructor.

### Named enumeration values

<a id="const-04"></a>

**CONST-04.** An enumeration is a normal library-defined value type with a finite
set of accepted values and named constants. Its native literal constructor
interprets symbols/bytes and rejects unknown representations; `=` provides
independent value copying. It does not add an integer-literal category, automatic
numbering, or an `enum` keyword. Value compatibility follows the full module type,
not equality of internal numeric tags. Define domain enumeration constants in the
shared section beside the using module's behavior, or in a constants-only namespace.
That set exists once without an instance pool; selecting a value does not execute
or copy the namespace. Public tag mappings are library contracts.

For example, a library palette may expose color values as follows:

```text
@=
red
..color
 |red

blue
..color
 |blue

```

A caller declares compatible storage and obtains one value:

```text
@!
selected color
..color

..palette
 red >> selected color
```

Paths/member names here are illustrative library names, not reserved English
words. Literal payloads are interpreted by the library type, not as source
operators. Built-in list methods are selected directly under LIST-04; enumeration
constants do not choose list actions.

### Using a qualified constant directly

<a id="const-05"></a>

**CONST-05.** A qualified constant of a concrete nongeneric namespace may be used
directly wherever a copy source is accepted: a declaration initializer, an
assignment source, or an entry-call input.
Its spelling is the full logical module path followed by `.` and the constant's
declaration name without surrounding dot spaces, for example `..palette.red`.
The final component is always
the constant name; all preceding components form the exact module path. For a
constant collection the member's existing `[]` storage suffix is retained, with
ordinary collection compatibility/cardinality rules. No longest-prefix lookup,
implicit import, wildcard search, or fallback to an executable module is allowed.
Preserve the root kind when splitting the path: `..palette.red` names constant
`red` in standard-library module `..palette`. The root marker is not a component;
a qualified constant source needs at least one module component and one member
component, and cannot use bare `.` or `..` as its namespace.
This compact dotted form carries no static binding block and cannot identify a
specialization of a generic module. It does not accept a placeholder namespace:
`...type.member` remains one atomic placeholder name under TPL-01, not member
selection. To copy a constant from a specialization or a substituted type, use
CONST-03's explicit namespace read into local storage, then use that local value
where an ordinary copy source is needed. Do not add an inline binding syntax or
select a specialization from the destination's expected type.

#### Resolving a qualified constant

Resolve the namespace and `@=` member statically, then copy the value under its
declared type directly into the existing destination or interface storage.
No implicit Ama variable, local alias, hidden instance, body execution, or new
literal category is introduced. A type-path line in a declaration remains a type
path; source position determines qualified-constant interpretation. Namespace
loading uses the same project/library/submodule mapping and collision checks.
For creation provenance this is the fixed-Type namespace reuse defined by
[TPL-11](templates.md#tpl-11), with the same policy as CONST-03 without static
outputs. This changes no constant-value initialization dependency or validation
of the namespace itself.
Missing namespaces/members, draft namespaces, incompatible types, and incorrect
storage forms are source-located compile-time errors.

Qualified constants are read-only sources, not receivers, invocation/resumption
targets, mutable-reference acquisition operands, or `%` continuation-copy sources.
Literal entry inputs follow CALL-07 independently of qualified constant copying.
Named/indexed condition syntax is unchanged. Existing explicit path-plus-`>>` constant reads remain available,
including when a program deliberately wants a local value to reuse or modify.
Argument ordering, named/positional consistency, `%` input prohibition and empty
positional placeholders remain unchanged.

#### Examples

Examples using illustrative library identities:

```text
# Supply a constant without declaring a local alias.
paint !
 < ..palette.red

# Initialize a declared value from a qualified constant.
selected color
..color
 ..palette.red

# Assign a different named value later.
selected color
 ..palette.blue
```

A qualified source in `@=` adds an explicit initialization dependency. It never
turns module initialization into an executable call or permits cycles of
constant-value initialization. Finite nominal type-reference cycles are checked
separately; they are not themselves constant-value dependencies.

### Constants whose types depend on parameters

<a id="const-06"></a>

**CONST-06.** A shared declaration may use local Type or Template parameters
introduced by its module's `@<`/`@>` interface under
[TPL-02](templates.md#tpl-02). Collect that interface before resolving the
declaration even though `@=` is written first. Apply the ordinary static
type-expression rules to its type line and static application block. Static
result aliases obey [TPL-12](static-results.md#tpl-12)'s source-order visibility;
interface collection does not make later aliases forward-visible. A shared
declaration introduces no new static parameter, and neither its initializer nor
its type may introduce a new placeholder that the interface does not declare.
Any omitted residuals of a named template type have the ordinary declaration
constraints under TPL-08; their required concrete types must be determined
before construction.

Resolve dependent declarations for each required concrete specialization.
Their types, storage forms, construction, literals and copy sources must satisfy
CONST-01/CONST-02 and the ordinary declaration rules after substitution. Typed
initializer sources contribute the equalities of TPL-03; a literal has no
intrinsic type and cannot choose a type parameter. An invalid dependent constant
rejects that specialization even if its body never reads the constant. A
substituted type is not automatically constant-capable merely because it is a
valid input, result or list element elsewhere.

Constant values may depend on literals, permitted initialized constants and the
selected concrete types' ordinary constant-capable default construction. They
must not depend on a runtime argument's value or liveness, instance fields,
retained results, execution position, mutable external state, or executing an
Ama body. Reading a constant through a static type namespace never supplies a
runtime value to its owner. Capability checking does not infer an otherwise
unknown type by looking for a constructor that accepts the initializer.

Keep one immutable shared part per concrete specialization per execution context
under STATE-03/TPL-05. Equivalent complete substitutions use the same constants
in that context; different substitutions have different shared parts, even when
their constant values happen to compare equal. Runtime instances keep independent
owned state. Constant construction and disposal remain context lifecycle
operations under CONST-02, not compile-time evaluation or per-instance work.

**Example: complete `.container`.** Assume the selected element type supports
independent value copying and otherwise satisfies CONST-01:

```text
@=
empty list[]
...element type

@<
element&
...element type

@!
```

The declaration owns an empty list whose element type is the interface's Type
parameter. No element value is required to construct it, but the selected
constant type must still satisfy the constant capability rules. Specializing
with `..integer` and `..text` yields separate typed empty lists. Two ordinary
instances of the integer specialization share its one constant, while keeping
their input references and other instance state separate. CONST-03 shows how
to obtain an independent working copy without first creating either instance.

**Example: dependent literal, declaration fragment.** Assume `...value type`
is introduced in this module's interface:

```text
@=
initial value
...value type
 |0
```

`|0` is interpreted by the selected type's literal contract. It does not imply
`..integer`, an additive identity, a minimum value, or any other algebraic
property. Literal construction follows LIT-04: native rejection during shared
initialization is a technical failure that prevents root startup; an equivalent
documented validation mechanism may diagnose it earlier. Use explicit named
constants such as a library's `zero`, `minimum` or `maximum` when those library contracts express
the intended meaning.

**Why this rule exists.** A generic container can publish an empty value of its
own element type, and an algorithm can explicitly select a supplied type's
neutral values or range bounds through CONST-03. These are two separate actions:
declaring a constant in the current specialization and selecting a constant
owned by another concrete type. They do not establish a parent/child relation
between those types.

An explicit namespace keeps the owner visible when several type parameters
provide names such as `zero` or `maximum`. Automatic imports or inheritance would
require conflict and hiding rules and could change unqualified name lookup when
a library adds a constant. Ordinary unqualified names keep their local meaning
under [NAME-04/NAME-06](#name-04), while member selection checks the chosen owner's
actual shared interface. A constant's descriptive name does not prove algebraic laws; those
remain part of the selected library's contract.
