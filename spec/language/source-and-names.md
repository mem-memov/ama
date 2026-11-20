# Source files, names, and layout

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Files and module paths](#files-and-module-paths)
- [Relative paths and local abbreviations](#relative-paths-and-local-abbreviations)
- [Complete names, including names with spaces](#complete-names-including-names-with-spaces)
- [Indentation and block boundaries](#indentation-and-block-boundaries)
- [Comments and blank lines](#comments-and-blank-lines)
- [Exact source spelling](#exact-source-spelling)
- [Module sections and unfinished files](#module-sections-and-unfinished-files)

<!-- /chapter-contents -->

A source file defines an ordinary module or a static module. Names can use your
own language and can contain spaces; punctuation and exact indentation make their boundaries visible.
This chapter explains how a physical file receives a logical module name, then
how the reader distinguishes names, comments, and nested blocks.

Start with the path examples below. The complete token inventory and recognition
order are in [Exact source spelling](#exact-source-spelling); the machine-readable
productions are in the [grammar](../grammar.md).

## Files and module paths

<a id="src-01"></a>

**SRC-01.** One extensionless source file defines one module. Its logical path
comprises a namespace kind and an ordered sequence of components separated by
adjoining ASCII slashes `/`. A single leading `/` selects the ordinary package
namespace. Two leading slashes `//` select the standard-library namespace.
These are logical addresses; their leading slash does not select the operating
system's filesystem root. The storage directories `packages/`, `dependencies/`
and the configured standard-library root do not occur in logical names.

An ordinary full module name has the structure
`/vendor/package/.../vMajor.Minor.Patch[-variant]/src/module/...` or the same
structure with `tests` in place of `src`. There is at least one package
component and at least one module component. Vendor, package and module
components use SRC-03 complete names. The version is one component under
[PKG-01](../project/packages.md#pkg-01): for example, `v2.4.14` or
`v2.4.14-speed`. Its dots are internal to that component and do not delimit
module names. Package discovery and version placement follow that canonical
package layout; a source path is not a dependency
wildcard selector and never contains `*`.

For example,
`packages/acme/shop/v2.4.14/src/item` has the full logical name
`/acme/shop/v2.4.14/src/item`. A companion C implementation is the sibling
`item.c`. An active `@C` module requires sibling `.h`, `.c` and `.abi` files for
native declarations/layout, implementation and protocol bindings respectively.
Optional capabilities are optional records, not permission to omit mandatory
lifecycle metadata. Ama implementations, static modules and constants-only
namespaces do not require native companions. `.abi` files are compiler inputs
describing native contracts, not executable Ama modules. A module's optional
`.ini` companion declares local path aliases under the
[package alias contract](../project/packages.md#pkg-04).

For a standard-library root at `stdlib/modules`, the source
`stdlib/modules/integer` is named `//integer`; the standard namespace needs no
vendor, package, version, `src` or `tests` wrapper. Required native companions
remain `integer.h`, `integer.c` and `integer.abi` beside that source. The prefix
does not reserve English words or translate names. Most library module names
may use any accepted spelling, including multiword Unicode names. Specific
interfaces may fix a public nominal identity: LIST-04 requires `//integer`
for list positions and lengths. That identity is an explicit library contract,
not a general rule for path spelling.

The full logical identity is `(namespace kind, component sequence)`. Resolve
only within the selected namespace, with no ordinary/standard fallback. A
template path identifies its source definition. A concrete specialization also
includes its static bindings under [TPL-05](templates.md#tpl-05); no extra
physical source file is required. Relative and alias spellings normalize to
this same identity under SRC-06 and do not introduce another type or instance.

### Keeping a module and its submodules together

<a id="src-05"></a>

**SRC-05.** A module's submodules live in a sibling directory named after its
source file with the ASCII suffix `+`. The marker belongs to filesystem layout,
not to a logical identifier. For example:

```text
packages/acme/shop/v2.4.14/src/
    item
    item.c
    item.h
    item.abi
    item+/
        validate
        calculate
        names
        validate+/
            details
```

| Physical module source | Logical module path |
|---|---|
| `packages/acme/shop/v2.4.14/src/item` | `/acme/shop/v2.4.14/src/item` |
| `packages/acme/shop/v2.4.14/src/item+/validate` | `/acme/shop/v2.4.14/src/item/validate` |
| `packages/acme/shop/v2.4.14/src/item+/calculate` | `/acme/shop/v2.4.14/src/item/calculate` |
| `packages/acme/shop/v2.4.14/src/item+/names` | `/acme/shop/v2.4.14/src/item/names` |
| `packages/acme/shop/v2.4.14/src/item+/validate+/details` | `/acme/shop/v2.4.14/src/item/validate/details` |

The same physical mapping applies within the standard-library root, preserving
its prefix: `stdlib/modules/item+/validate` maps to `//item/validate` when
`stdlib/modules` is that root and its sibling `item` source exists.

Ordinary directories such as `shop/` group namespace components. A `name+/`
directory marks submodules of its sibling extensionless `name` module file; that
file must exist, although normal draft rules still determine whether the parent
itself participates in compilation. Nesting does not implicitly import or execute
the parent, inherit its declarations, or establish runtime instance ownership.
A parent may be a constants-only, static or executable module, or an unused draft;
a referenced child must independently satisfy active-module rules.

Strip exactly the structural trailing `+` from each such directory component
when deriving logical identity. Use the full logical identity for type checking,
hashes, and duplicate detection; reject conflicting physical sources instead of
preferring a search path. `+` is not added to module-path grammar. Source maps,
`#line`, includes and filesystem diagnostics retain actual physical paths.

Derive native companions from the physical source filename, so the implementation
for `/acme/shop/v2.4.14/src/item/validate` is
`packages/acme/shop/v2.4.14/src/item+/validate.c`, with `.h` and `.abi` beside it.
The convention lets an extensionless parent file and its child directory coexist
without changing existing top-level companion names.

### A type path does not create an instance

<a id="src-04"></a>

**SRC-04.** A module path identifies a source definition, not a callable object.
An ordinary definition provides a runtime type; an empty static definition
provides a nominal marker under [TPL-14](static-results.md#tpl-14). A nonempty
static definition provides a graph producer whose named ordinary Type export
may be selected under
[TPL-12](static-results.md#tpl-12). A static producer itself cannot own runtime
storage or be invoked/resumed. Executable module types
have no implicit/default live instances. A module's `@=` section is shared
immutable storage, not an implicit executable instance. Ordinary source-level execution,
resumption, and retained-result reads must select an explicit instance through
an owned name, a reference, or an indexed element. A whole-list operation selects
a declared collection under its collection contract, not a type. Instance and
collection declarations may be explicit or receiver-inferred under TYPE-06.
CALL-09 also permits selecting declared result storage of an enclosing operation;
it does not create a default instance or expose private fields.

### Resolving a path without shadowing

<a id="src-02"></a>

**SRC-02.** Select the complete package version from `packages/` or
`dependencies/` under the [package precedence rule](../project/packages.md#pkg-02)
before searching its modules. Do not merge their file trees or use an ignored
external copy to fill a missing local module. Duplicate complete module
identities within the selected source set MUST be rejected. The ordinary `/`
and standard `//` namespaces do not collide merely because some component
bytes coincide.
Filesystem aliases must not create conflicting identities, and the same physical
module must not be exposed under different logical identities, including through
roots of different kinds.
Module lookup must diagnose missing modules and executable references to drafts.
The compiler must define which directory entries are source candidates and must
not treat generated outputs or companion `.c`, `.h`, `.abi` or `.ini` files as
Ama modules. Dependency staging in `download/` is not a module source root.

## Relative paths and local abbreviations

<a id="src-06"></a>

**SRC-06.** A relative module path starts with `./` or `../`. Resolve it against
the logical directory containing the source module that writes the reference,
not against that module's own name, the compiler's working directory, a caller,
or a specialization's consumer. Its namespace kind is the defining source's
namespace kind and cannot change during normalization. Thus a relative standard
path remains within `//`; a relative ordinary path remains within `/`.

Split a path into components before looking up any files. Process components
left to right with a stack: `.` leaves the stack unchanged; `..` removes its
last component; any other component is appended. Attempting to remove a
component from an empty stack is an error. Do not load, require or execute an
intermediate component that a later `..` removes. Absolute paths admit the same
whole-component `.` and `..` normalization after their namespace prefix.
In a module-reference context the normalized endpoint must be a complete
module identity satisfying SRC-01; qualified constant sources instead split
the final component under SRC-03 and CONST-05.
Empty components, repeated slashes except the initial `//`, and a trailing
slash are invalid. A third leading slash is invalid.

Module references may instead start with `~` followed immediately by one
complete alias name, optionally followed by `/` and a component sequence.
The alias name ends at the first slash. Resolve aliases by their exact scoped
name under the [package alias contract](../project/packages.md#pkg-04), then normalize
the expanded path and suffix using the same stack algorithm. Normalization
must not cause a second alias lookup. The alias target determines the
namespace kind. A suffix may contain whole `.` or `..` components, but cannot
cross that namespace's root. An alias never changes canonical module identity.

Relative paths can cross module, `src`/`tests`, version, package and vendor
boundaries when the normalized target is valid. The direction restrictions for
dependencies from `src` to `tests` are checked against that final identity under
the [package test-isolation contract](../project/packages.md#pkg-09), including
references written through aliases or static expressions. Passing a test type
through an ordinary template parameter does not manufacture a source-written
dependency in the template's definition.

**Example: the containing directory is the base.** Suppose these sources exist
inside one package version:

```text
src/
    tree
    compare
    tree+/
        traverse
        check
tests/
    verify tree
```

In `src/tree+/traverse`, `./check` denotes its sibling `check`, `../compare`
denotes `src/compare`, and `../tree` denotes `src/tree`. In `src/tree`, its child
is `./tree/traverse`. In `tests/verify tree`, `../src/tree` selects the tested
module. In `src/tree`, `../tests/verify tree` normalizes successfully but is a
forbidden source-to-test dependency. No reference spells the physical `+`.

**Why this rule exists.** Relative addresses shorten paths while retaining one
meaning after loading, independent of process state. Stack normalization
terminates after one step per supplied component and has one endpoint. A
module's aliases are configuration for that same address calculation, not
alternate modules or declaration-dependent guesses.

## Complete names, including names with spaces

<a id="src-03"></a>

**SRC-03.** Ama source and native `.abi` companions must be well-formed UTF-8.
Decode Unicode scalar values, rejecting malformed sequences instead of treating
arbitrary bytes above 0x7F as letters. Module and variable names are case-sensitive
and compared byte-for-byte, without normalization, transliteration, or case folding.
A name consists of one or more nonempty name words separated by exactly one
ASCII space U+0020. Each word starts with `_` or a Unicode `XID_Start` character
and otherwise continues with `_` or `XID_Continue` characters, using the identifier
properties defined by [Unicode UAX #31](https://unicode.org/reports/tr31/).
Each word may additionally contain isolated internal ASCII hyphens `-` (U+002D).
Every hyphen must immediately follow an ordinary character of that word and be
immediately followed by `_` or an `XID_Continue` character. It cannot begin or
end a word, adjoin another hyphen, or adjoin a space or structural separator.
Several separated hyphens are permitted: `red-blue-green` is one valid word.
The hyphen does not begin a new word; `item-2` is valid. A space does begin a new
word, whose start rule still applies: `item step2` is valid, but `item 2` is not.
These rules support international alphabets, including Chinese, without requiring
C-compatible names.

The same profile applies to variable and constant names, runtime interface/member
names, ordinary module-path components and the descriptive components of static
names under [TPL-01](templates.md#tpl-01). Only U+002D is added as a hyphen;
similar-looking punctuation is not substituted for it. All other scalars retain
their existing Unicode identifier-property requirements. Underscores remain
ordinary start/continuation characters, including next to a hyphen: `item-_value`
and `item_-value` are valid. An allowed combining continuation character remains
allowed after a hyphen; this rule introduces no additional Unicode filtering.

In a name position, `a-b` is one complete name, not subtraction. This extension
adds no arithmetic operator or alternative name-splitting rule. Invalid spellings
such as `a--b`, `a- b` and `a -b` are diagnosed rather than split into shorter
declared names. Literal/comment payload and the standalone destructor marker
`-` in a native `.abi` record retain their own rules. Authored C function names
in those records still follow the target C identifier contract, not the Ama
complete-name profile.

A name MUST NOT begin or end with a space. Empty words, repeated internal spaces,
TAB, and other Unicode whitespace inside a name are invalid. Structural indentation
and token separators lie outside the name span; they are not silently trimmed
from an otherwise invalid name. The complete name, including its single internal
spaces and hyphens, is the unit of identity and lookup. `result` and `result name`
may coexist as distinct base names; reading the latter never searches for the
shorter name.

Use Unicode 17.0.0 identifier properties and record that edition with the compiler
source. Identifier acceptance must not depend on the host locale or a silently
updated system Unicode library. Generating/vendoring those tables is an
implementation task; changing the accepted property edition is a reviewed source
contract change, not an implicit library upgrade.

Each ordinary component of a module path uses the same complete-name rule;
the structurally identified version component has the separate SRC-01 spelling.
Source notation places `/` directly against its adjacent components:
`/acme/shop tools/v2.4.14/src/item state` contains the complete names `shop tools`
and `item state`. No space may split a namespace prefix, adjoin a slash, or
pad a component. Relative whole components `.` and `..` have only the SRC-06
normalization meaning. Dots elsewhere in concrete paths occur only in the
version component; they are not module or constant-member separators.

Exactly one leading `.` followed immediately by a valid identifier start
introduces a static name in a permitted position under
[TPL-01](templates.md#tpl-01). Its complete descriptive spelling is a nonempty
sequence of SRC-03 identifiers separated by adjoining single dots, for example
`.data.element type`. The entire sequence denotes one scoped name: there is no
filesystem lookup, member selection or automatic relationship to `.element type`.
Slashes, empty components, a trailing dot and two or more leading dots are
invalid in static names. The slash in `./module` and the second dot and slash
in `../module` distinguish those relative paths lexically from `.module`.
This static spelling also covers graph input/result names, static aliases and
named binding labels under [TPL-12](static-results.md#tpl-12). It needs no
corresponding file or directory. The independent annotation `@...` remains the
static-module marker.

Indentation or an operand separator before a path or static prefix is outside
that name, as in `< //palette/red`, and retains its ordinary grammar rule.
In a qualified constant source, expand aliases and normalize the path first,
then take exactly the final component as its constant name under CONST-05; the
preceding identity must denote its module. Type-line paths instead denote
their complete module. This source-context split does not try alternate module
prefixes. Physical names beginning/ending with a space or containing
repeated internal spaces or invalid hyphen placement are not valid Ama components;
diagnose rather than trim, collapse, replace spaces/hyphens with underscores,
or guess another path. `item-name`, `item_name` and `item name` are distinct
logical spellings and are never aliases of one another. The structural
physical `+` suffix follows SRC-05 independently of spaces in the component name.
Filesystem lookup must preserve a one-to-one mapping between parsed components
and the module file.
Adapt native path encoding explicitly without normalizing or merging identities.
Reject a project whose distinct source paths the platform cannot distinguish,
including case/normalization aliases; do not silently rename or combine modules.
A path/name that cannot be represented unambiguously on the platform is diagnosed
as an unsupported path, not reinterpreted under the host locale.

### Examples: one name or two components?

These are spelling examples, not complete programs.

| Source text | Interpretation |
|---|---|
| `total price` | One name containing two words |
| `total price&` | The reference form of the complete name `total price` |
| `/acme/shop tools/v2.4.14/src/item state` | A module path containing the complete names `shop tools` and `item state` |
| `items[current position]` | List `items`, indexed by the complete name `current position` |
| `item-2 next_step` | One complete name; the digit continues the first word |
| `red-blue-green` | One word with two separated internal hyphens |
| `_item-_value`, `item_-value` | Valid combinations of ordinary underscores and isolated hyphens |
| `/acme/shop-tools/v2.4.14/src/item-state` | A path preserving internal hyphens exactly |
| `.данные.тип-ключа` | One static name; no corresponding file is required |
| `item-name`, `item_name`, `item name` | Three distinct complete names |
| `total  price` | Invalid: two spaces inside a name |
| `/acme/shop tools/v2.4.14/src/ item` | Invalid: a space next to a path separator |
| `.data/type` | Invalid: a slash inside a static name |
| `..data`, `...data`, `.data.` | Invalid static spellings |
| `-item`, `item-`, `item--name` | Invalid: leading, trailing or adjacent hyphens |
| `item- name`, `item -name`, `item - name` | Invalid: a hyphen cannot adjoin a word-separating space |
| `item−2`, `item–2` | Invalid: U+2212 minus and U+2013 en dash are not ASCII hyphens or identifier characters |

**Why this rule exists.** A space within a name does not separate operands.
Structural punctuation or a line boundary ends the name; an allowed internal
hyphen is part of the name. Parsing never depends on which shorter names have
already been declared. The same source keeps the same shape as declarations are
added or removed. One shared profile makes a variable, a module-path component
and a type placeholder equally readable. Isolating each hyphen permits familiar
compound names while catching incomplete or accidentally repeated punctuation.

**Why the grammar is unambiguous.** A hyphen is outside the ordinary continuation
class. The name-word production consumes each hyphen together with its following
ordinary continuation character; other continuation characters are consumed alone.
Every accepted word consequently starts and ends with an ordinary character and
has no adjacent hyphens. Conversely, every word satisfying those conditions and
the stated start/character rules has exactly that grouping. Single spaces then
determine the word boundaries without consulting declared names.

## Indentation and block boundaries

<a id="syn-01"></a>

**SYN-01.** Ama uses exactly one ASCII space U+0020 per nesting level: a code line
at level `n` has exactly `n` initial spaces. Repeated initial spaces are structural
indentation, not repeated separators. Parameter bindings, result bindings, and
selected-result operations are exactly one level deeper than their operation.
All branch/loop/alternative bodies are one level deeper than their marker;
in particular, `:` and `{` have their bodies on following lines at level `n+1`.
They require no trailing space and admit no inline body. A closing `}` is at the
opening `{` level. A control body following `?`, `:`, or `{` may be empty,
including a body containing only blank lines or comments. For each written
marker whose body contains zero statements, the compiler MUST issue one warning
at the marker's source location without rejecting the program. Count statements
before lowering or optimization; declarations count as statements. This warning
also applies to unreachable bodies, conditional alternatives, and call/read or
selected-operation refusal handlers. An omitted optional body has no marker and
requires no empty-body warning. An empty module implementation after `@!` remains
valid under SYN-03; it is not a control body covered by this warning.

Body extent follows indentation, never an inference about intended code. After
a marker at level `n`, consume complete statements beginning at level `n+1`,
with comment-only lines at that same body indentation under SYN-02. Empty blank
lines do not affect nesting. A code or comment-only line at level `n` or shallower,
or EOF, ends an implicit indentation body when the enclosing syntax permits
continuation there. A closed body cannot resume deeper statements after a
parent-level comment. Any enclosing construct must still have its required
markers. In particular, before the loop's closing `}`, standalone comments must
remain in its body or a valid nested body, not at the closing marker's level.
Thus a following same-level statement can leave the preceding body empty and
trigger its warning. Skipping an indentation level remains invalid; an empty
body does not absorb misindented code.
Unexpected indentation is an error, never name padding or discretionary layout.

TAB U+0009 is forbidden throughout Ama and `.abi` input, including indentation,
code, literals, comments, and drafts. LF and CRLF are supported; a lone CR is
invalid. Blank lines contain only their line ending and MUST NOT delimit sections.
The last physical line need not have a line ending. For grammar recognition only,
treat a nonempty unterminated final line as terminated by one virtual LF; it is
not a source byte or part of a literal. Empty files remain empty. Preserve actual
bytes and positions for literals and diagnostics. The global TAB prohibition is
checked before activation/draft exclusion or native literal construction. Code
spacing follows SYN-06; `.abi` records additionally follow PROTO-11.

## Comments and blank lines

<a id="syn-02"></a>

**SYN-02.** `#` introduces a comment outside literal payload. A comment inside a
literal is data. The parser must preserve literal trailing spaces and distinguish
structural indentation from payload bytes.
In a nonliteral line containing code, the first `#`, the maximal contiguous run
of zero or more spaces immediately before it, and the remaining comment text
form one trailing comment suffix. Those prefix spaces are excluded from code
spacing and from any name; their count may be arbitrary.

An Ama comment-only line has structural indentation instead: `#` must directly
follow exactly the number of spaces required by its containing body or other
structural region. A module-level comment has zero indentation; a comment in
a body, argument block, or initializer block has that block's indentation.
Extra alignment spaces before `#` are invalid. A comment cannot introduce a
deeper block by itself. A dedented comment can belong to an enclosing region
only where the grammar permits that region to continue; later child lines
cannot reopen the ended block. Comments between a condition and its first
`?`, `:`, or `{` align with that condition. Parent-level comments may precede
an optional `:` alternative or refusal handler. A loop's closing `}` has no
such parent-comment gap; comments before it belong to the loop body.

Comments and blank lines still count as zero statements, so a body containing
only correctly indented comments is empty and warns under SYN-01. Repeated
spaces are allowed in comment text and trailing comment prefixes; the global
TAB ban still applies. A code line without a comment has no trailing spaces.
A line containing only spaces is invalid rather than blank. Flat native `.abi`
comments follow PROTO-11 rather than Ama block indentation.

```text
ready
{
 # Loop body comment
 worker !
  # Result binding comment
  > value
}
```

## Exact source spelling

<a id="syn-06"></a>

**SYN-06.** Source whitespace and token boundaries are exact syntax. Outside
structural indentation, accepted literal payloads, comment text, and trailing
comment prefixes, required code separators are exactly one U+0020. Standalone
comment indentation must satisfy SYN-02. Module paths and
qualified constant sources use adjoining slashes under SRC-03. Static names
use adjoining dots under [TPL-01](templates.md#tpl-01). Storage suffixes
`&`, `[]`, and `&[]` directly follow the complete name. A reference-forming prefix
`&` directly precedes its owned access. Index delimiters `[`/`&[` directly follow
the list name, directly precede the complete index access, and end with an
adjoining `]`. No space is permitted at those joins or immediately inside the
brackets. Each multiword name still has exactly one internal U+0020.

All other grammatical separators remain exactly one U+0020, including those
before execution suffixes `!`/`%`, after a continuation-copy prefix `%`, and
around binding markers. Thus `target& !`, `% target&`, and `input& < &item` have
required spaces outside the compact access. Indentation and an operand separator
before a path, static name or reference prefix are also outside the access.
There is no optional whitespace, repeated separator, trailing code space, TAB,
or alternative Unicode spacing character. Repeated spaces are permitted only in
the three excluded regions, never within a name or between code tokens.

In Ama, the fixed compound tokens are `@...`, `@=`, `@<`, `@>`, `@!`, `@C`,
`>>`, `[]`, `&[]`, `&[`, the standard namespace prefix `//`, and the relative
path prefixes `./` and `../`.
They contain no internal whitespace.
Recognize the longest fixed token at a punctuation position; `&[]` is one token,
not `&[` followed by `]`. At a type start, distinguish `//`, `/`, `../`, `./`,
`~` and a single-dot static name before consuming components. Once a path has
started, recognize whole `.` and `..` components under SRC-06; do not interpret
them as static names. A bare `.` is not a static name, and a bare namespace
prefix is not a module path.
The remaining source punctuation tokens are `.`, `/`, `~`, `&`, `[`, `]`, `!`, `%`, `<`,
`>`, `|`, `?`, `:`, `{`, `}`, and `^`; `#` begins the comment region.
Outside name words, code punctuation must be one of these tokens; `_` remains
allowed by the name-word rule. The `.abi` token inventory and flat comment-line
layout follow PROTO-11; encoding, line endings, trailing comment suffixes, blank
lines, and the TAB ban are shared.

| Source form | Meaning |
|---|---|
| `worker !` / `worker %` | Entry / resume |
| `target& !` / `target& %` | Execute / resume through a reference |
| `&item` / `% item` / `% target&` | Reference formation / owned continuation copy / copy through a reference |
| `&` as a value source | Reference to the current instance of the containing module |
| `items[]` / `targets&[]` | Owning-list / reference-list name |
| `items[current position]` / `targets&[current position]` | Owned / reference element selection |
| `/acme/shop tools/v2.4.14/src/item state` | Module path with multiword components |
| `.data.element type` | One descriptive static name, without filesystem lookup |
| `./check`, `../compare` | Relative module paths |
| `~containers/dictionary` | Exact scoped path alias followed by a suffix |
| `input name < source name` / `output name > receiver name` | Named bindings with complete multiword names |

Literal `|` is followed immediately by its raw payload, not an implicit separator;
any space after it is data. In `< |payload`, the single space between `<` and `|`
is syntax. A standalone marker ends the code part without a trailing space.
Annotation tokens occupy their own zero-indent lines. Required spaces around
separate operation/binding markers are grammar separators, never leading/trailing
spaces in names. Path slashes, static-name dots and storage/access punctuation adjoin their operands
without spaces.

### Recognition order

Apply lexical recognition in this exact order:

1. Decode UTF-8, reject TAB globally, and establish physical line boundaries under
   SYN-01. These checks also apply to drafts and `.abi` companions.
2. Recognize a literal only in a permitted initializer/input position, then keep
   everything after `|` as payload. Otherwise recognize the first `#`. If code
   precedes it, separate the trailing comment suffix and its preceding space run
   under SYN-02. On a comment-only line, keep all initial spaces as structural
   indentation for the parser to validate. Never cut literal data at `#` or
   validate its repeated spaces as code.
3. Separate the initial structural indentation from the code part. A line with
   no code/comment is blank only if it has no spaces. Validate code spacing and
   tokenize fixed compounds and complete names without trimming or repairing input.
4. In each name position, consume the maximal sequence of valid name words joined
   by single spaces, stopping at reserved punctuation or the end of the code part.
   Two independent names must have a structural separator; a space alone never
   separates two operands. Parsing must not consult declared names or try shorter
   name splits. Validate complete structure and exact nesting in active modules.

After region separation, every name boundary is determined by punctuation or
the code-part boundary, each internal word separator is exactly one space, and
the indentation prefix is outside all names. This invariant yields one name
segmentation independently of declarations, loading order, or runtime types.
Whitespace outside the explicitly exempt regions is rejected, not normalized.
This also applies recursively to indices: `items[positions[current row]]` has
one unique delimiter structure, and `&item states[current position]` forms a
reference to the selected owned element. Compact punctuation never splits or
normalizes the single spaces inside either multiword name. A qualified constant
list source likewise retains its compact suffix, as in
`/example/demo/v1.0.0/src/catalog/item states[]`.

## Module sections and unfinished files

| Annotation | Meaning |
|---|---|
| `@=` | Shared immutable constant declarations |
| `@<` | Runtime parameter declarations |
| `@>` | Runtime result declarations |
| `@!` | Executable Ama body |
| `@C` | Native companion implementation |
| `@...` | Complete static Type/Template equation graph |

<a id="syn-03"></a>

**SYN-03.** Annotations occur at indentation zero. An ordinary module's section
order is optional `@=`, optional `@<`, optional `@>`, then one implementation
selector, `@!` or `@C`.
`@=` is a declaration section, not an implementation selector; it may accompany
either implementation. Repeated, unknown, or out-of-order annotations in an active
file are errors. `@!` may introduce an empty body. Only layout follows `@C`;
constant declarations follow `@=` until the next section or end of file.

A file containing only `@=` and its constant declarations/layout is a complete
constants-only module, with no executable implementation or instance pool.
An empty `@=` section is allowed. Parameter/result sections require `@!` or `@C`;
adding `@=` must not make an unfinished interface without implementation valid.

A static module instead consists of optional leading layout, the single marker
`@...`, and its graph under [TPL-12](static-results.md#tpl-12). It cannot combine
`@...` with `@=`, `@<`, `@>`, `@!` or `@C`. Its graph contains compile-time
Type/Template expressions, not runtime declarations, initializers or executable
statements. An empty graph declares a static marker under
[TPL-14](static-results.md#tpl-14); a nonempty graph is a producer. Neither has
an instance pool or can be an executable entry.

### Drafts and activation

<a id="syn-04"></a>

**SYN-04.** A file with none of `@=`, `@!`, `@C`, or `@...` is a draft excluded from
compilation. It may contain unfinished declarations without full parsing, name
binding, or type checking. UTF-8 decoding, the lexical whitespace rules in SYN-06,
and recognition of genuine zero-indent section markers still apply; text inside literal payload/comments cannot activate
a file. Companion files cannot make a draft executable or trigger layout/stub
output.

Using a draft as a type or program entry is an error identifying the file and
referring location. Adding `@!`/`@C`, `@...`, or `@=` for a constants-only file, activates
full validation. An incomplete constant declaration then fails rather than being
ignored as a draft. Constants-only files are valid dependencies, but cannot be
program entries or invocation/resumption targets. They do not create a hidden
executable instance. Typed enumeration values and selection follow CONST-01
through CONST-05. Editor draft checks do not change
build status. An intentional empty executable body uses `@!`.

<a id="syn-05"></a>

**SYN-05.** Only the section annotations specified above are accepted. There are
no alternate implementation selectors or separate constructor section.

### Example: a small complete module

This complete module owns one integer field. Its body has a declaration and no
executable call; construction initializes `count` to zero.

```text
@!
count
//integer
 |0
```

The type line aligns with the name. Its initializer is one level deeper. An
implementation may instead be intentionally empty by containing only `@!`.

For the meaning and lifetime of declared fields, continue with
[Declarations and scope](declarations.md). For `?`, `:`, and `{` bodies, see
[Conditions and loops](control-flow.md).
