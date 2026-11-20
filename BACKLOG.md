# Ama product backlog

This file collects future changes and open design questions before they become
selected engineering work. An entry records an intention or a proposal; it does
not change the [specification](spec/README.md), authorize editing it, or request
implementation. The specification remains the source of truth.

Use [TODO.md](TODO.md) for selected work with prerequisites and acceptance
conditions, [DONE.md](DONE.md) for completed records, and
[AMBIGUITIES.md](AMBIGUITIES.md) for ambiguities blocking an active task.
Allocate task IDs across BACKLOG, TODO and DONE without reuse. Move an agreed
item into TODO with its existing ID and decisions; do not leave a second task
copy here. Follow the [development standard](docs/development/README.md).

## Reference notation

<a id="t222"></a>

- [ ] **T222 — Require a space after the reference-formation operator.**
  Status: backlog only; recorded from the user's 2026-11-20 request. No
  specification or implementation change is authorized by this backlog entry.
  Prerequisites: none; independent of T221.
  Desired spelling: exactly one ASCII space between prefix `&` and its owned
  operand: `& item`, `& items[position]`, and `input& < & item`.
  The reference suffix stays adjoining (`target&`); standalone self-reference
  `&` has no operand and gains no trailing space. Reference semantics stay intact.
  Current contracts still require the joined prefix under
  [REF-01](spec/language/values-and-references.md#ref-01),
  [SYN-06](spec/language/source-and-names.md#syn-06) and the
  [grammar](spec/grammar.md#accesses-and-sources).
  Steps when separately authorized: align those rules, examples and acceptance
  scenarios; update recognition and affected source tools/fixtures using source
  structure, preserving literal/comment data; document and verify the change.
  Accept: test one-space formation with simple, multiword and indexed operands;
  reject joined operands, repeated spaces, TAB and a line break in place of the
  separator. Preserve suffix/self-reference parsing and all reference behavior.
  Complete component/composition, source-tool and conformance/documentation
  checks before marking this language change implemented.

## Standard-library internationalization

<a id="t223"></a>

- [ ] **T223 — Design localized standard-library names without replacing the compiler.**
  Status: open design question; recorded from the user's 2026-11-20 request.
  The request authorizes this backlog record, not a specification or compiler
  change. Prerequisites: none for discussion; preserve canonical nominal
  identities and compiler-recognized library roles in any chosen design.

### Purpose and proposed direction

Allow a project to select a standard library with names in its own language
while using the same compiler executable. An illustrative mapping is:

```text
..integer -> ..целое
```

The canonical `..integer` identity is required by
[LIB-01](spec/language/library.md#lib-01). The translated path illustrates the
localization goal and remains a proposal; it introduces no alternative identity
or implementation claim.
The goal includes compiler-recognized library roles, beyond choosing Unicode
names for ordinary user modules.

The user's proposed mechanism is a translation/mapping file supplied among the
project's dependencies. The compiler would discover that file and interpret each
entry's position as identifying its meaning. The resource location, filename,
format, position numbering and discovery rules remain undecided. This is a
candidate to evaluate, not an accepted compiler contract.

Current constraints to review are [SRC-01](spec/language/source-and-names.md#src-01)
and [SRC-02](spec/language/source-and-names.md#src-02) for rooted nominal identity
and conflict-free resolution, [LIB-01](spec/language/library.md#lib-01) for
validated library mappings, and [LIST-04](spec/language/lists.md#list-04) for the
fixed `..integer` position/length type. Replacing library files alone does not
redefine these contracts. Specialization identities also depend on complete
Type identities under [TPL-05](spec/language/templates.md#tpl-05).

### Decisions required before implementation

1. **Stable meaning of a mapping.** Compare position-based entries with explicit
   stable role identifiers. Inserting or reordering positional entries can assign
   a different meaning to the same name; decide how versions, reserved positions,
   missing entries and malformed resources are checked before accepting either
   representation. A translated name alone must not establish a trusted role.
2. **Resource discovery.** Define which dependency locations participate, whether
   the project selects a resource explicitly, and what happens when none or
   several are present. Require deterministic resolution and explicit conflict
   handling rather than search-order guesses.
3. **Type identity.** Decide whether translated paths are aliases of one canonical
   module or distinct nominal types. Define equality, specialization keys and
   coexistence of dependencies written in different languages accordingly.
4. **Translation boundary.** Inventory compiler-recognized paths and interface
   member names, including intrinsic list services and native contracts. Separate
   their semantic bindings from diagnostics, documentation and editor-language
   translation, and decide which surfaces this feature covers.
5. **Validation and distribution.** Define required mappings, duplicate names,
   conflicting roles, accepted resource versions and signature/protocol checks.
   Explain how a language pack is shipped or replaced without recompiling the
   compiler and without silently changing a program's meaning.

### Expected outcome of a future design task

Provide an agreed resource format and selection model, examples with two
languages and mixed-language dependencies, and an explicit identity policy.
Compare positional and stable-key formats before choosing one. Define rejection
cases for incomplete, reordered, conflicting or incompatible resources, and
acceptance tests for identical behavior and preserved type checks with the same
compiler executable. Only then plan specification changes and implementation.

### Record of this backlog update

Created this file at the user's request and transferred T222 from TODO with its
original ID and complete pending record. Updated workflow/navigation to keep
future ideas, selected work and completed evidence in separate files with one
shared ID sequence. This update changes neither language requirements nor the
compiler; T222 and T223 remain unfinished. Documentation navigation, unique task
IDs and generated-reference input fingerprints are the checks for this update.

## Relative module paths and directory relocation

<a id="t224"></a>

- [ ] **T224 — Add relative module paths and support them in directory relocation.**
  Status: backlog only; recorded from the user's 2026-11-20 request. The request
  authorizes recording this future change, not specification or implementation
  changes. Prerequisites: agree on relative-path syntax and resolution rules
  before implementation; assess interaction with [T223](#t223).
  Purpose: shorten references to nearby modules without changing their intended
  targets. Earlier candidates `./a`, `../a`, and `../../a` remain proposals;
  this entry does not select their syntax or interpretation.
  Design: define the resolution base, permitted parent traversal, root boundaries,
  and interaction with existing `.`/`..` roots, `...` placeholders and physical
  `name+/` storage. Specify how relative and rooted spellings resolve to module
  identities under [SRC-01](spec/language/source-and-names.md#src-01) and
  [SRC-02](spec/language/source-and-names.md#src-02).
  Required companion work: extend the directory relocation utility under
  [TOOL-01](spec/implementation/tools-and-conformance.md#tool-01) through
  [TOOL-04](spec/implementation/tools-and-conformance.md#tool-04) to resolve and
  rewrite relative module references. Recompute affected paths when the referring
  module, its target, or both move, including references from outside the moved
  folder. Preserve the resolved dependency graph under the planned identity
  mapping, even when the target's identity stays unchanged but its relative
  spelling must change. Use resolved source structure rather than text replacement;
  retain preview, preflight, byte-preservation and recoverable-transaction behavior.
  Accept when separately authorized: align specification and grammar, implement
  shared resolution in the compiler and source tools, and test mixed rooted and
  relative references, nested/Unicode/multiword paths, `name+/` storage, all three
  importer/target move combinations, moves of their common ancestor, root-boundary
  cases and rejected unresolved/conflicting rewrites. Verify that relocation
  preview and apply preserve intended targets and leave literals/comments intact.
