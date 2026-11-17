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

An Ama name identifies storage. Its type is a module, and its suffix says whether
that storage owns a value, holds a reference, owns a list, or holds a list of
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
is a concrete module path or a scoped placeholder permitted by
[TPL-01/TPL-02](templates.md#tpl-01). A path naming a template is specialized
statically under [TPL-03/TPL-04](templates.md#tpl-03), including when it types a
nested field. Optional initializer lines are one space deeper. The storage marker
directly adjoins the name; it never belongs to the type line and never has an
intervening space.

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
its root creation/execution under CONST-01 through CONST-05, never once per instance
or invocation. Earlier constants in the same module may be used by name in later
constant/interface/body initializers.

Shared constants are excluded from instance layouts, interface reset, ownership
trees, ordinary copying, and `%` continuation copying. Creating, copying, or
resuming an instance cannot duplicate or mutate them. They have no busy state,
slot generation, caller link, or execution position. They cannot be assignment
receivers or sources of mutable references. Shared storage is not an instance
pool or a callable singleton.

A constants-only module provides the shared part without executable instances.
It requires no pool and cannot run as the root. A module path followed by `>>`
reads shared constants under CONST-03, never an instance's retained results.

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

**Explanation.** An assignment source or a declared result already determines a complete type. Ama lets the first textual occurrence of a receiver use that information instead of repeating a type line. The suffix remains explicit so that ownership is visible.

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
   its grammar. Collect all explicit module declarations and resolve their
   complete module-type identities. This inventory is available throughout the
   module for ordinary lookup and for FLOW-05's foreach-header lookup. Collecting
   a later explicit declaration does not count as encountering its name at an
   earlier source position and does not suppress an earlier receiver-inferred
   declaration. If both declare that base name, report a duplicate.
2. Traverse the parsed module once in increasing source-byte order, visiting each
   branch, loop, and `:` body at its written location. Maintain the set of
   encountered module-variable base names, the types of earlier inferred
   fields, and the active lexical foreach bindings. All four storage spellings
   share a base name. A possibly multiword name is one occurrence, never a set
   of occurrences of its individual words.
3. At a foreach header, resolve the source list from the explicit typed inventory,
   earlier inferred fields, and enclosing lexical bindings. Its element type is
   therefore known before applying FLOW-05's exact binding rule. Reuse a compatible
   visible reference or introduce the specified local reference for that body.
   New local-binding declarations and occurrences resolved to those locals do
   not enter the module-variable encountered set. An assignment/output receiver
   resolved to such a local updates that cell and cannot declare a module field.
4. For every other module-variable occurrence, inspect the encountered set before
   adding its base name. A simple assignment or output receiver whose base name
   is absent declares an inferred field. Other occurrences are ordinary uses or
   explicit declarations according to their syntax. Add each encountered base
   name, including unresolved uses. An earlier unresolved use therefore prevents
   inference at a later receiver.
5. Type each new inferred field immediately, before processing any later source
   construct. An assignment's following source line supplies its complete module
   identity and transfer storage form through the table below. Resolve names in
   that line using explicit declarations, earlier inferred fields, and visible
   foreach bindings; also visit their occurrences in source order. A dependency
   on the field currently being inferred is an error. For an output receiver,
   resolve the operation target and selected interface position or label. Take
   exactly that result's declared full module identity and storage form. Constant
   reads use the selected `@=` declaration; list methods use LIST-04's fixed typed
   result interface. Neither source execution, runtime values, nor inspection of
   a callee body contributes a type.
6. Require the receiver's written storage form to match the source transfer form
   or selected result exactly. Check namespaces, initialization, ownership,
   capabilities, and transfer compatibility. Missing or mismatching `&`/`[]`
   is an error; do not insert a suffix, dereference, form a reference, or convert
   between scalar and list. After a field has been typed, add it to the inventory
   of earlier inferred fields used by later constructs. A valid later explicit
   declaration can resolve ordinary uses under the existing visibility rule; a
   later inferred receiver cannot. An invalid first occurrence never triggers a
   search for a different declaration site or a different inferred type.

Module-path components, qualified constant member names, callee-side binding
labels, direct-result selector base names, comments, and literal payloads are not
local variable occurrences. They do not enter the encountered set. Names inside
an index and a local constant used by its unqualified name do enter it. At an
indexed receiver, visit its collection and index uses; the receiver cannot declare
either name. Classify header/source occurrences before entering a foreach body's
new local scope. These rules keep the traversal lexical and unambiguous.

### Deriving the storage form

Assignment-source inference uses the following exact rules. An indexed owned
element or reference-list entry is scalar; a whole-list source retains its list
form. The module identity is determined by the source's declared type, never its
current runtime contents.

| Assignment source | Inferred module identity | Required receiver storage form |
|---|---|---|
| Owned scalar access or scalar constant | Source module type | Owned scalar |
| Reference access, including an indexed reference-list entry | Referent module type | Reference scalar |
| `&item` or `&items[index]` | Selected owned instance's module type | Reference scalar |
| Standalone `&` | Containing module's complete identity | Reference scalar |
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

The root target's type must be supplied by an explicit declaration or an earlier
receiver-inferred declaration, or by a visible foreach binding. A CALL-09
selected target follows the explicitly
typed result interface of its enclosing operation; each finite selector path is
resolved from its root without searching bodies or caller names.
Interface declarations in `@>`/`@=` have explicit module types; intrinsic list
method result types are supplied by LIST-04's validated interfaces.
They do not depend on inferred caller fields. Any field needed to type an
operation target or assignment source must have an explicit type, come from
an earlier receiver-inferred declaration, or resolve to a visible foreach
binding whose element type is supplied by its earlier header. Indexed operands
must likewise resolve to valid typed declarations. Although an assignment source follows its receiver
in the text, dependencies on other inferred declarations strictly decrease their
receiver source positions. These edges terminate at explicit types, fixed
interfaces, the typed list declaration supplying a foreach binding, or the
known containing-module type of `&`, making inference well-founded. Explicit
declarations may occur later and terminate the dependency argument; only
dependencies on inferred declarations must strictly decrease source positions.
Reject missing types and self/cyclic inference dependencies;
do not guess or infer backward from a later receiver. Existing visibility of
valid explicit declarations, cross-module loading, initialization-order, and
eager-construction checks remain separate.

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
within each module and reject cycles. Qualifying a local forward reference does
not bypass that ordering. Required constant dependencies initialize first, before
the root can start.
Native construction must not introduce hidden initialization dependencies.
Do not execute an Ama module body or arbitrary native constructors at compiler
time. Build-time validation is allowed only through an explicitly equivalent,
documented validation mechanism. A runtime initialization failure identifies the
module/declaration and prevents root startup; clean completed constants and partial
construction through the lifecycle protocol. Dispose successful shared storage
once after executable instances, in reverse initialization order.

#### Exact initialization order

Plan initialization over individual constant declarations, not over a graph that
merely says one module mentions another's type. Add an ordering edge from each
constant to the preceding constant in its own module, and a dependency edge to
each constant value its construction reads, including declared owned-field
initializers. Reject cycles in this graph. Choose the next dependency-ready
declaration deterministically by the bytewise full module path, then declaration
ordinal. A module declaring constants of its own executable value type is not
thereby cyclic: shared `@=` storage is not an owned field of that type. Eager
owned-state construction cycles are checked independently under TYPE-04.

### Reading another module’s constants

<a id="const-03"></a>

**CONST-03.** Read constants of another module with a module-path target and
ordinary saved-read bindings:

```text
..palette
 red >> selected color
```

The compiler statically resolves the module and the requested `@=` members.
Named requests use constant declaration names; positional requests follow their
order in `@=`. Existing read rules require at least one `>>` with a receiver,
consistent named/positional mode, and no `<`, `>`, `!`, or `%`. This is a static
constant read, not entry/resume, an instance acquisition, or a computed getter.
A path cannot expose parameters, body fields, or instance results. Missing modules
or members, draft sources, incompatible types and mutable receivers in shared
storage are compile-time errors. Runtime value-copy failure remains technical.
An instance target with `>>` continues to expose only its retained `@>` results,
not constants. Local constants are available directly by their declared names.

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

**CONST-05.** A qualified constant may be used directly wherever a copy source is
accepted: a declaration initializer, an assignment source, or an entry-call input.
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

#### Resolving a qualified constant

Resolve the namespace and `@=` member statically, then copy the value under its
declared type directly into the existing destination or interface storage.
No implicit Ama variable, local alias, hidden instance, body execution, or new
literal category is introduced. A type-path line in a declaration remains a type
path; source position determines qualified-constant interpretation. Namespace
loading uses the same project/library/submodule mapping and collision checks.
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
turns module initialization into an executable call or permits dependency cycles.
