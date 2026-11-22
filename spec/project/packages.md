# Projects, packages, and repeatable module delivery

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a project is a collection of named experiments](#explanation-a-project-is-a-collection-of-named-experiments)
- [Project storage and complete module names](#project-storage-and-complete-module-names)
- [Choosing one copy of a package version](#choosing-one-copy-of-a-package-version)
- [Small configuration files](#small-configuration-files)
- [Package and module path aliases](#package-and-module-path-aliases)
- [Dependency selectors and version variants](#dependency-selectors-and-version-variants)
- [Connecting a logical prefix to a repository](#connecting-a-logical-prefix-to-a-repository)
- [Delivering a module with its companions](#delivering-a-module-with-its-companions)
- [Keeping the experiments with the code](#keeping-the-experiments-with-the-code)
- [Source dependencies and test substitutions](#source-dependencies-and-test-substitutions)
- [Publishing downloads and taking ownership](#publishing-downloads-and-taking-ownership)

<!-- /chapter-contents -->

## Explanation: a project is a collection of named experiments

An Ama project can contain code maintained in its own repository, downloaded
code, or both. A project that assembles other people's modules does not need a
token local implementation. The same layout supports work on several versions
of an algorithm at once: one can emphasize speed, another memory consumption,
and a third clarity. Their complete names keep those experiments distinct.

The useful unit of delivery includes the evidence needed to check the code.
Tests are small programs that exercise their selected modules. Keeping those
programs with a downloaded module lets another developer repeat the experiment
instead of relying on an author's claim that it passed elsewhere. This supports
the confidence model in [CORE-06](../introduction.md#core-06); a finite passing
test suite still does not prove every possible behavior.

**Example: physical project tree.** These vendor and package names are
illustrative. `v2.4.14-speed` and `v2.4.14-memory` are distinct package versions.

```text
project/
├── packages/
│   └── acme/
│       └── algorithms/
│           └── sorting/
│               ├── v2.4.14-speed/
│               │   ├── package.ini
│               │   ├── src/
│               │   │   ├── sort
│               │   │   └── sort+/
│               │   │       └── partition
│               │   └── tests/
│               │       └── sort/
│               │           ├── empty input
│               │           └── partition/
│               │               └── repeated keys
│               └── v2.4.14-memory/
│                   ├── package.ini
│                   ├── src/
│                   └── tests/
└── dependencies/
    └── acme/
        └── containers/
            └── v1.3.0/
                ├── package.ini
                ├── src/
                └── tests/
```

The first `sort` module has the logical name
`/acme/algorithms/sorting/v2.4.14-speed/src/sort`. Its child is
`/acme/algorithms/sorting/v2.4.14-speed/src/sort/partition`. The containing
`packages/` directory and structural `+` do not become parts of either name.
The test `empty input` has the distinct name
`/acme/algorithms/sorting/v2.4.14-speed/tests/sort/empty input`.

This chapter defines project layout, configuration and source delivery.
[Source paths](../language/source-and-names.md#src-01) define path recognition
and normalization; [project tools](../implementation/project-tools.md) define
the separate commands that prepare a project. Acquisition delivers existing
identities; explicit [source copy and movement](../implementation/tools-and-conformance.md#directory-relocation-utility)
can create new identities under their chosen dependency policy. These are
target contracts. Examples and task records do not claim that the compiler or
tools already implement them.

## Project storage and complete module names

<a id="pkg-01"></a>

**PKG-01.** A project root has two optional source-storage directories:
`packages/` for code maintained as part of the project and `dependencies/` for
externally acquired code. Either may be absent, including `packages/` in a
project composed entirely from dependencies. An empty project is valid as a
scaffold; a compilation still requires the selected source module and its
required closure to exist.

Both storage directories use this structure:

```text
<storage>/<vendor>/<package component>[/<package component>...]/<version>/
    package.ini
    src/
    tests/
```

There is exactly one vendor component and at least one package component.
Package components form an ordinary hierarchical namespace. A version is one
component spelled `vMAJOR.MINOR.PATCH`, optionally followed by `-VARIANT`.
The three numbers are nonnegative decimal integers with no leading zero except
the number `0`. The variant is a nonempty name word under
[SRC-03](../language/source-and-names.md#src-03); it has no spaces and may use
that rule's Unicode letters, underscores and isolated internal hyphens. The
version is not a range expression, and `+` build metadata is not part of its
spelling. The exact productions are in the [grammar](../grammar.md).

A component matching the version production marks the version boundary, so
vendor and package components must not themselves match that production.
The directory immediately inside a package version identifies `src` or `tests`.
Both are ordinary components of complete module identities. They are not
implicitly inserted or omitted in source paths or dependency selectors. The
vendor is the first ordinary logical component. Physical storage names
`packages`, `dependencies` and temporary `download` are excluded from identity.
The physical submodule mapping in [SRC-05](../language/source-and-names.md#src-05)
applies below `src` and `tests`.

Every available package version has its own `package.ini`, `src/` and `tests/`.
The last two may be empty while a package is being authored. Required tests for
delivery are governed by PKG-08. A partially acquired version may contain only
the selected module units and their closure; it must retain its original
`package.ini` and the structural ancestors needed to resolve those identities.

The standard-library namespace `//` is separate and is configured explicitly
under [SRC-01](../language/source-and-names.md#src-01); it is not another
vendor searched under the two ordinary source-storage directories.

## Choosing one copy of a package version

<a id="pkg-02"></a>

**PKG-02.** A package-version identity comprises the vendor, ordered package
components and complete version component, including any variant. For each
identity, a version present under `packages/` takes precedence over the same
version under `dependencies/`. This choice applies to the entire version.

After choosing `packages/`, resolution must ignore every file of the external
copy. A missing local source, test, configuration entry or companion is not
filled from `dependencies/`. Report a missing required input in the selected
version. Two different versions remain distinct, even if their contents happen
to be identical; the module sets of successive versions need not overlap.

This is a storage-selection rule, not general module shadowing. Conflicting
physical definitions within the selected copy remain errors under
[SRC-02](../language/source-and-names.md#src-02). Other configured roots cannot
add a second competing definition. Source lookup must not use directory order,
modification times, guessed release precedence or whichever candidate happens
to compile.

**Why this rule exists.** Copying an external version into `packages/` gives
the project one coherent version to maintain. Filling holes from the external
copy could silently change a locally edited algorithm when a download changes.

## Small configuration files

<a id="pkg-03"></a>

**PKG-03.** `package.ini` records required external selections, shared path
aliases and repository connection settings. It must not repeat the vendor,
package name or version already encoded in its directory path, and must not
declare a default executable. Any suitable ordinary module can be selected as
an executable by the compiler's explicit entry selection; a test is selected
in the same way as an independent microprogram.

Ama uses a defined, small INI profile rather than relying on parser-specific
INI behavior. Configuration files are well-formed UTF-8, without a byte-order
mark or tab characters. LF and CRLF line endings are accepted, and the last
line need not have a line ending. Blank lines and lines beginning with `;` are
ignored. Comments occupy complete lines; `;` inside a value is literal data.
All nonblank lines start in column zero and have no trailing whitespace.

An assignment is `key = value`, with exactly one ASCII space on each side of
`=` and nonempty key and value. A value is the remaining line, without quoting,
escaping, interpolation, continuation lines or environment-variable expansion.
An `=` later in the value is data. Names and paths obey their own lexical
rules, including meaningful single internal spaces. Section headers have the
exact forms below. Their order is immaterial; each section and each key within
its section may occur only once. Unknown sections, unknown fixed repository
keys, duplicate keys and malformed entries are errors.

The optional `[dependencies]` section contains one selector per line, without
an assignment or invented numerical key. Its selectors are defined by PKG-05.
The optional `[aliases]` section contains the assignments defined by PKG-04.
Each `[repository /prefix]` section contains the fixed keys defined by PKG-06.
An empty configuration file is valid when none of these entries is needed.

**Example: complete `package.ini`.**

```ini
[dependencies]
/acme/containers/v1.3.0/src/dictionary
/acme/algorithms/*/v2.4.14-*/src/mod1

[aliases]
~containers = /acme/containers/v1.3.0/src
~own code = ./src

[repository /acme]
url = https://git.example.org/acme/ama.git
directory = packages/acme

[repository /acme/algorithms/sorting]
url = https://git.example.org/acme/sorting.git
directory = ama
revision = main
```

A module's optional companion configuration is the physical source filename
with `.ini` appended, such as `process.ini` beside `process`. It uses the same
line/assignment profile, contains only sectionless alias assignments, and has
no dependency or repository sections. Configuration extensions identify file
formats rather than source-language keywords; alias names remain international.

## Package and module path aliases

<a id="pkg-04"></a>

**PKG-04.** An alias key is `~` immediately followed by a complete ordinary name
under [SRC-03](../language/source-and-names.md#src-03), including its permitted
single internal spaces. The key contains no slash. `~` is mandatory in both the
configuration key and source use. Alias names are case-sensitive and compared
by the same exact-name rule as ordinary names; no prefix or approximate match
is permitted.

A package alias belongs to the version whose `package.ini` defines it. It is
available to all source and test modules of that version, including its static
modules, and in those modules' `.ini` companions. It does not leak into a
different version, another package or a consuming module's scope. A module
alias belongs only to the corresponding module; physical submodules do not
inherit it. A local alias key that equals an available package alias key is an
error, as are duplicates within either table. There is no alias shadowing.

A package alias's right side must be an absolute ordinary path, an absolute
standard-library path or a relative path. It cannot contain another alias.
A module alias's right side may additionally start with a package alias from
its own version. It cannot start with any module alias, including itself.
Thus alias expansion has at most two table lookups and cannot form a cycle.

A relative package-alias target is resolved from its package-version directory.
A relative module-alias target is resolved from the logical directory containing
the corresponding module. Both use the path normalization rules in
[SRC-06](../language/source-and-names.md#src-06); the compiler process's current
directory is irrelevant. A normalized alias may denote a module or namespace
prefix; declaring it does not import, execute or instantiate a module. Resolve
the final module or constant only in its actual source-use context; the alias
table itself does not guess that the final component must be a module.

In source, the alias name ends at the first `/` or the end of the path operand.
Look up that entire name, append any remaining complete path components to its
target, and normalize the combined path. The resulting root kind and components
determine identity exactly as for a written-out path. Unknown aliases are errors.
An alias does not create another nominal type, source module or specialization.

**Example: package aliases and one module's local aliases.** With the
`package.ini` above, `process.ini` may contain:

```ini
~dictionary = ~containers/dictionary
~helpers = ./helpers
```

An ordinary declaration fragment in `process` can then use:

```text
values
~dictionary
 .key type < //text
 .value type < //integer
```

The package alias `~containers` is also directly usable in that module as
`~containers/dictionary`. The static binding names `.key type` and `.value type`
are scoped static names under [TPL-01](../language/templates.md#tpl-01), not
aliases. The illustrative dictionary constructor is assumed to declare those
named inputs.

## Dependency selectors and version variants

<a id="pkg-05"></a>

**PKG-05.** A dependency selector identifies ordinary package versions, source
modules or test modules. Its path retains the vendor and, for a module,
`src` or `tests`. A selector may start with an absolute ordinary path or a
package alias. Resolve the alias first. Dependency masks are configuration/tool
inputs; they are not valid module identities or source type paths.

Normalize an alias's real-path base before appending a selector suffix. The
resulting selector pattern must not contain a whole `.` or `..` component;
normalization must never cancel a wildcard. Unlike source paths, dependency
selectors are canonical patterns, so a structural traversal belongs in the
alias's real-path target rather than the pattern suffix.

A selector without `*` selects exactly its named existing version or module.
A selector ending at a version selects that version's complete source tree
and its associated tests. A module selector selects the module unit defined by
PKG-07. A selector must not implicitly select a latest version or convert an
unsuffixed version into its suffixed variants.

In a selector, a component that is exactly `*` matches a nonempty chain of
complete path components. An embedded `*` matches zero or more characters
within its one component, never `/`. The literal parts around it must match
exactly. Several matches yield a set of identities, not a choice of one
preferred result. Multiple matching decompositions produce one set member.
Only identities with the valid package/version structure of PKG-01 are eligible.
The match decision must use an exact finite sequence/string algorithm; the
term "greedy" does not authorize a search-order-dependent result.

**Example: selecting experiment variants across a package tree.**

```text
/acme/*/v2.4.14-*/src/mod1
```

This selects both of the following if they exist:

```text
/acme/graphs/v2.4.14-speed/src/mod1
/acme/algorithms/sorting/v2.4.14-memory/src/mod1
```

It does not select `v2.4.14` or `v2.4.15-speed`. A variant's suffix is an opaque
name, not a compiler promise about speed, memory use or quality. Different
versions may have different module trees. A wildcard-selected version without
the requested module contributes no match and is skipped; an empty match set
is valid. Compilation separately reports a missing module when an actual
required complete identity is unavailable.

Dependency entries declare which external selections a package makes available;
loading a module of that package does not eagerly copy or compile everything
matching every entry. An explicit dependency on another ordinary package-version
identity must be covered by the declaring version's selectors, regardless of
whether either version is stored in `packages/` or `dependencies/`. References
within the same version need no dependency entry. The separately configured
standard namespace `//`, when it is the dependency target, does not require
ordinary package selectors.

Coverage includes the main source and full structural subtree of a selected
module unit under PKG-07, not just the exact spelling of its main module. A
selected whole version covers its source tree. The associated tests retained by
PKG-08 are part of the selected delivery. A required external identity outside
these selections is an error. The compiler and acquisition tool use this same
rule on the prepared source view; checking coverage never authorizes a hidden
download. Missing wildcard members still contribute no selected identity.

Check dependencies where a definition explicitly writes them. A Type or
constructor supplied by a caller does not become a newly authored dependency
of the receiving template; preserve the test-substitution rule in PKG-09.

A definition in the standard namespace must not explicitly depend on an
ordinary Ama module. Check this restriction at the source definition that
writes the dependency, after alias expansion and complete path normalization.
It covers ordinary declarations and references, result types, constant
namespaces, static constructions, and source-specified native/compiler inputs
that name Ama modules. An alias or static expression cannot bypass the rule.
Diagnose the forbidden reference at its original source occurrence. Do not
borrow selectors or repository mappings from a caller or invent a package
configuration owner for the standard module.

Standard ordinary/static templates may receive ordinary concrete Types or
Templates through their existing typed parameter connections. Reusing or applying
such a supplied argument does not by itself create a newly authored ordinary
dependency in the standard definition. Preserve the originating connection's
definition and check its explicit dependency there. This permission does not
waive ordinary type/operation checks or the existing native-leaf restrictions.
An unused local alias with an ordinary target still imports nothing under
PKG-04; reject a forbidden dependency when an actual reference uses it.
This restriction concerns dependencies on Ama modules, not C headers, system
libraries or other native build inputs governed by the native/build contracts.

**Why the standard library is isolated.** Its modules have no ordinary vendor,
package version or `package.ini`. Keeping their explicitly named Ama dependencies
inside `//` makes their admission independent of which project calls them.
Generic parameters still let standard algorithms operate on project-defined
types without embedding a particular vendor or version into the library.

**Examples: interface fragments of a standard module.** This declaration is
forbidden even if the ordinary module exists in the prepared project:

```text
@<
value
/acme/pkg/v1.0.0/src/data
```

A standard template can instead declare a parameter type:

```text
@<
value
.value type
```

An ordinary caller may supply a value of
`/acme/pkg/v1.0.0/src/data` through this input. The explicit reference belongs
to that caller; its own dependency selectors and source/test isolation apply.
The standard template receives the inferred Type without acquiring that caller's
package scope or introducing a second written dependency.

An acquisition operation's explicit selector arguments are its initial seeds.
Needed external references and tests extend those seeds under PKG-08 using the
declaring package's selectors and repository mappings. This separates "this
dependency may be used" from "this particular operation needs every member"
and preserves deliberate all-matches acquisition.

## Connecting a logical prefix to a repository

<a id="pkg-06"></a>

**PKG-06.** A `[repository /prefix]` section maps a complete ordinary vendor/
package prefix to a directory in a Git repository. The prefix has one or more
whole components, starts with the vendor and ends before the version boundary;
it contains neither masks nor aliases. Its mandatory `url` identifies the Git
remote. Its mandatory `directory` is a slash-separated physical directory
relative to the repository checkout, or `.` for the checkout root. It must not
be absolute or escape that checkout. The optional `revision` is a branch, tag
or commit accepted by Git. Without it, use the remote's default branch.
No other repository keys are defined by this profile.

Repository mappings are scoped to the package version whose `package.ini`
declares them. Resolve an outgoing external dependency using the requesting
definition's own package configuration. Loading the target's configuration
governs that target's outgoing external requests; it never retroactively
changes where the target itself was obtained. Requests for further members
of that same version use its already established source snapshot, rather than
rerouting through its outgoing repository table. Do not merge all loaded
configurations into one global routing table. The client supplies the initial
seed mappings explicitly or
selects an existing configuration as the acquisition environment. Thus a first
acquisition can prepare a dependency-only project without requiring a local
package or inventing a mandatory root project manifest.

For a requested complete identity, select the matching prefix in that request's
environment with the greatest number of whole components. An arbitrary
character prefix is not a match: `/acme/data` does not match `/acme/database`.
Contradictory mappings for the same exact prefix in one environment are errors.
The selected source must supply the requested identity or report its absence.
Do not fall back to a shorter mapping after an unavailable remote, rejected
revision or missing input. A package version already selected locally under
PKG-02 uses that selected copy without consulting a repository to fill holes.

Remove the chosen logical prefix from the requested path and append its
remaining components to `directory`, applying the physical submodule mapping
under [SRC-05](../language/source-and-names.md#src-05). For example:

```text
logical identity: /acme/containers/v1.3.0/src/dictionary
repository:       https://git.example.org/acme/ama.git
directory:        packages/acme
physical source:  packages/acme/containers/v1.3.0/src/dictionary
```

The more specific `/acme/algorithms/sorting` mapping in the preceding example
instead maps `.../sorting/v2.4.14-speed/src/sort` to
`ama/v2.4.14-speed/src/sort` in the sorting repository. Here the ellipsis is
informative abbreviation, not source syntax.

Before reading a selected remote source, an acquisition operation resolves
its selected revision to a commit. All files read from that source during the
operation, including `package.ini`, companions and tests, come from that same
commit. Each identical remote/revision source is resolved only once per
operation, even if several logical prefixes route to it. Enumerate wildcard
candidates from these finite snapshots and apply the same longest-prefix rule
within the requesting environment to each candidate before including it.
Directory enumeration order cannot change the selected contents.

Different requests within the same package-version identity must agree on one
source contract, including requests for different modules of that version.
For acquired code, compare the exact repository URL, resolved commit and
physical package-version directory after mapping. Equivalent requests share one
acquisition record. Incompatible contracts are an error,
even if the current source bytes happen to agree; do not select whichever
request was processed first or guess that two different remote URLs are the
same repository. Distinct revision names resolving to the same commit do not
by themselves conflict. This check preserves one coherent origin and prevents
mixing one version's configuration, code and tests from different snapshots,
without letting a newly loaded dependency rewrite earlier routing decisions.

A package version names Ama code; a commit identifies a snapshot of its storage.
An external author can change code without renaming its version directory, so
the tools must not claim that version spelling enforces immutability. Acquisition
records the resolved commits and hashes of delivered files under
[PROJECT-03](../implementation/project-tools.md#project-03). Comparing a later
download with retained hashes can detect a change. No mandatory lockfile or
external release-enforcement service is part of this contract.

## Delivering a module with its companions

<a id="pkg-07"></a>

**PKG-07.** Selecting a module copies its extensionless Ama source file, all
same-module companion files, and the entire sibling `name+/` tree recursively.
Companions are files whose complete physical basename is the selected module's
basename followed by `.` and a nonempty suffix. They include `.ini`, `.c`, `.h`
and `.abi`, and are not limited to those known extensions. This includes the
optional [CMake companion](../implementation/application-builds.md#build-01),
which is delivered without executing it. Selecting a parent
does not discard an unused child or companion because a current execution does
not appear to need it. This is a structural source-delivery unit, not executable
code elimination.

The rule applies recursively to the complete selected subtree, including its
directories, module sources and companions. Distinguish active delivery units
from structural context records. An explicitly selected or transitively needed
module is an active unit, as is each module inside that unit's complete subtree.
Source selection also retains any ancestor main file and companions required
to make a physical `name+/` directory valid under SRC-05. If that ancestor has
not independently been selected or needed, it is a structural context record.
Its presence alone selects neither its other children nor its tests or semantic
dependencies. It need not pass executable admission: the SRC-05 rule permits
an unused draft parent for an active child.

If an actual resolved dependency or explicit selection later requires the
ancestor, promote it to an active unit. Then include its complete subtree,
dependencies and associated tests normally. The role follows explicit request,
resolved graph edges and physical ancestry, not an estimate of likely use.

For identity-preserving acquisition or promotion, preserve relative physical
names, file contents and package placement. Retain the original package-version
`package.ini`; do not rewrite it into a guessed summary of the current use.
If the exact file `README.md` exists at that same version root, also retain it
byte for byte with any selection from the version.
This is optional package guidance, not an Ama source/configuration input or a
condition for compilation. Absence is permitted, and tools must not invent an
instruction file or search other versions or ancestor directories for one.
Its presence/content is recorded as delivery/report metadata. Changing only
README guidance does not change the program's build identity. A supplied
non-file/unreadable entry at that exact guidance path is diagnosed as a delivery
I/O/shape failure, rather than silently published as a valid document.
Other files outside the structural unit are included only
when explicitly required by a resolved source/test/native/configuration input.
Do not guess resource ownership from nearby filenames or inspect arbitrary C
expressions to predict undeclared runtime file access. A required asset must be
identified by a supported explicit input declaration or supplied by the caller
as a separately identified external test input. Missing declared inputs are
errors; tools must not silently substitute an unrelated file.

For acquisition, only needed external code is materialized under the finite
closure of PKG-08. This does not weaken the complete-unit rule: selecting one
needed module can bring its whole submodule tree, including children that are
not on a particular call path. Files required to repeat tests are required
inputs even when they are unnecessary to the production executable.

Explicit source copying and movement under
[TOOL-01 through TOOL-04](../implementation/tools-and-conformance.md#directory-relocation-utility)
also preserve complete source units, companions and applicable tests, but may
create new module identities. Their exact destination map, allowed path rewrites
and dependency permission edits are defined by
[TOOL-02](../implementation/tools-and-conformance.md#tool-02). These operations
do not install an origin version's `package.ini` over an existing destination
version's configuration or merge origin alias/repository tables into its scope
when copying selected units. An explicitly selected whole-version tree carries
its own configuration; TOOL-02 defines its exact re-expression in the copied
version. Resolve
original definitions in their original scopes during preparation; check copied
definitions in the destination version's scope, preserving
[PKG-02](#pkg-02), [PKG-04](#pkg-04) and [PKG-06](#pkg-06).

## Keeping the experiments with the code

<a id="pkg-08"></a>

**PKG-08.** Every active delivered `src` module has an associated test directory
below the same package version's `tests/`. Remove the `src/` prefix from the module's
logical suffix and use that component sequence as the ordinary test-directory
path. The test directory contains independent test microprograms, their module
units, and any explicitly required fixtures. A selected source module without
its required test directory or any associated test program is an acquisition
error. Static-only modules use ordinary microprograms that exercise compile-time
composition; absence of a runtime instance does not waive testing. Negative
compiler/conformance cases remain subject to the compiler's own rejection-test
contract; this association rule does not silently classify malformed programs
as independently executable positive tests.

For a test program to belong directly to a source module, its main source is
an immediate entry of that module's associated test directory. Nested source
modules have nested associated directories. Each selected source module must
have at least one directly associated test program; a test only for a child
does not by itself provide the parent's tests.

A directly associated test root is a concrete executable ordinary or native
module: all of its own type parameters must be determined, and the module must
be admissible as an independent executable entry under the normal compiler
rules. An open module template, static module, constants-only definition or
draft is auxiliary material, not a test root. Retain such helpers with the
associated directory, but do not count them toward the required concrete test.
If no concrete directly associated test exists, acquisition fails. Determine
these roles from the language's semantic classification, never from filenames
or by suppressing compiler diagnostics. Syntax and type errors in definitions
subject to checking remain errors; invalid programs must not be reclassified as
helpers merely because independent compilation fails.

Determine concreteness after the ordinary inference of
[TPL-03](../language/templates.md#tpl-03). The mere presence of an original type
placeholder does not make a module an open helper: for example, a typed default
can uniquely determine it. Insufficient information for the selected root's own
parameters distinguishes a genuinely open template from a concrete entry with
an invalid or unresolved body. Preserve that distinction without interpreting
diagnostic strings or trying guessed type arguments.

**Example: a concrete test may use an open helper.** An associated directory
can contain a concrete executable `check` and an ordinary template `helper`
whose parameter has type `.T`. `check` supplies the concrete type when it uses
`helper`. Both files are delivered; only `check` counts as a direct test. A
directory containing only the open `helper` does not satisfy this requirement.

**Example: association uses logical source components.**

| Physical source | Associated physical test directory |
|---|---|
| `src/sort` | `tests/sort/` |
| `src/sort+/partition` | `tests/sort/partition/` |
| `src/sort+/partition+/pivot` | `tests/sort/partition/pivot/` |

Ordinary directories in this mirrored test tree have no `+` suffix. A test
program can itself have submodules, which use the ordinary `name+/` rule inside
its own test directory. Deliver the entire associated test directory, so the
experiments written for that source are retained together. Every source module
included because of an active unit's complete subtree or a needed source
dependency gets the same test treatment. A topology-only ancestor context
record under PKG-07 does not become an experiment target solely because its
source file is retained. A test helper under `tests` does not recursively
require a separate tree of tests for itself.

Compute delivery as the least set closed under these operations:

1. Add the explicitly selected module/version units and their structural inputs
   under PKG-07.
2. For every active included source module, add its associated tests and declared
   test inputs; retain all inputs necessary to compile and execute those tests.
3. Resolve references made by active source and test definitions, including
   ordinary and static type expressions, constant namespaces, native companions
   and configuration paths that actually supply required inputs. Add the
   required module units and their applicable package configurations.
4. Repeat for newly added identities until no operation adds an input.

An active complete identity is expanded only once; promoting a previously
retained context record schedules its one active expansion. Each record can
only move from absent to context to active, never back. For a finite reachable
graph of repository snapshots and their finite entries, this monotonically
growing finite state reaches a fixed point; source-dependency cycles do not
require an infinite download. This termination argument assumes that reachable
source graph is finite and that transport requests finish or report failure;
it does not prove that an arbitrary sequence of newly supplied remote sources
is finite. Test association applies to active source modules, avoiding an endless
tests-of-tests expansion. Apply PKG-02 before resolving each version's contents
and PKG-09 before admitting each explicit dependency.

The required module set follows the compiler's resolved dependency relation
under [DEP-03](../implementation/compiler.md#dep-03), not text search or guessed
call reachability. Open Ama templates remain distributed as source under
[TPL-13](../language/templates.md#tpl-13). Inspecting definitions needed to
resolve an operation may require reading additional snapshot metadata; only
the required delivery units and evidence belong in `dependencies/`.

Each test is compiled and run with its tested modules and actual dependencies,
not by automatically linking every source module in the package. Repeating the
same selected experiment requires the recorded source snapshots, test inputs
and declared environment/toolchain assumptions. A test report must distinguish
missing external prerequisites, compilation rejection, test failure and success;
an unavailable experiment must not be reported as a passing one.

## Source dependencies and test substitutions

<a id="pkg-09"></a>

**PKG-09.** A definition under `src` must not explicitly depend on a module under
`tests`, whether in its own package or another package. The restriction covers
ordinary references, constant namespaces, declarations, static constructions
and source-specified native/compiler inputs. Resolve aliases and normalize the
complete identity before checking its structural `src`/`tests` component.
Relative paths, aliases and static producers cannot bypass the restriction.
An unused alias declaration alone does not import its target; each actual source
reference must still pass this check.

Definitions under `tests` may depend on `src` and `tests`. A test may pass its
own concrete test type to a Type parameter of an ordinary template under `src`,
or provide a test-defined constructor through the static composition interface
that permits it. The originating explicit dependency is in the test definition.
The resulting test specialization may use that type; it is not rejected merely
because its source template resides in `src`. The restriction is on the source
definition's explicit dependency, not a blanket ban on concrete substitutions
originating in tests.

```text
src/algorithm ────────────────→ src/storage contract
       ↑
       │ instantiated and called by
tests/algorithm/failure case ─→ tests/helpers/failing storage
```

**Why this rule exists.** Production definitions must remain usable when test
code is removed from an execution build. Test code still needs to inject
controlled failures, unusual states and observable implementations into generic
algorithms. Tests are mandatory for delivery under PKG-08; they are not a hidden
production runtime dependency.

## Publishing downloads and taking ownership

<a id="pkg-10"></a>

**PKG-10.** Source acquisition is an explicit project-tool operation under
[PROJECT-01](../implementation/project-tools.md#project-01), not an implicit
network effect of compilation. If the source transport supports partial reads,
the tool may retrieve the selected units directly. If it cannot, use a private
operation directory below `download/` to obtain the complete remote package or
repository snapshot, extract the selected units and closure into prepared
dependency storage, and remove that operation's temporary download after
successful publication. The transport may still download more bytes than the
final delivery; selective delivery does not promise partial-network support.

Concurrent operations use disjoint temporary directories. Cleanup removes
only the current operation's temporary files and must not delete another
operation's work. `download/` is excluded from module lookup, package priority
and compilation. A cancelled or failed acquisition must not expose half a
module unit or mixture of snapshots as a usable selected version. Prepare and
validate complete deliveries before publishing them; report conflicts rather
than overwrite unrelated or locally maintained files. Recovery must distinguish
prepared inputs from published inputs and preserve previously usable contents.

An explicitly requested external refresh may replace its own prior delivered
copy after snapshot and closure validation. It must report recorded hash
differences when prior hashes are available; it cannot prove that an author's
unchanged version spelling means unchanged code. Publication, provenance and
failure acceptance are specified in
[PROJECT-03](../implementation/project-tools.md#project-03).

To take control of external code, copy the required package-version contents
from `dependencies/` into `packages/` and save them in the project's own
repository. That version then follows PKG-02. To preserve all code needed for a
repeatable build or test experiment, also preserve the required transitive code,
tests, explicit inputs and source evidence. Copying only one module does not
freeze an unrecorded external dependency. Promotion is an explicit operation;
compilation must not create a private modified copy automatically.
