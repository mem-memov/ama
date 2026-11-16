# Checked documentation workflow

`make docs` builds the file/interface reference with Doxygen. Python tools supply
read-only input views and check the result; they do not write API reference pages.
The canonical specification remains the maintained `spec/` tree.

Install Doxygen and the pinned source-position parser with
`python3 -m pip install -r docs/requirements-documentation.txt`. Select executables
with `DOXYGEN` and `DOCUMENTATION_PYTHON`. The bootstrap evidence uses Doxygen
1.11.0, markdown-it-py 3.0.0, Python 3.14, GCC 15.2 and GNU Make 4.4.1.

## Commands and their limits

- `make docs-check` checks the current per-file registry, reviewed purpose and
  contract records, real C test-case interfaces and standalone ready headers.
- `make docs` also generates and checks actual HTML destinations, canonical
  chapter placement, maintained file pages and XML function/type descriptions.
  Every Doxygen warning is a build failure; warnings are never an accepted debt.
- `make docs-reproducible` performs two complete generations and requires identical
  file membership and SHA-256 bytes for every output, including the tag file.
- `make docs-complete` checks the complete per-file partition and fails while any
  registered debt remains; generation is the separate `make docs` command.
- `make docs-clean` removes the generated site, staged views, reports and warnings.
- `make -C tests/unit/documentation` runs the independent C/assert contract leaf.

`ready.json` maps each reviewed file to a purpose, contract, named C tests and
executable commands. `debt.json` assigns every other current audited input to a
specific TODO owner, records its exact fingerprint and preserves T001's known
preprocessor consumers. Those consumers are leads for review, not automatically
claimed behavioral coverage. A successful bootstrap check prints outstanding
debt explicitly. It does not certify the incomplete implementation.

The registry partitions the VCS's tracked and unignored maintained inputs. It
covers C, headers, include fragments, Python/shell, Make inputs, native ABI, `.exports` library-build configurations and
project fixtures, plus the original audit's interface/expectation inputs. Build,
`generated/` and Python cache trees are excluded. Deleted, new or changed debt
entries require explicit review; the checker never blesses files automatically.
When a task changes a debt file, either close its purpose/contract/test gap or
update its fingerprint and explanation with the still-open owning task. Never
silently copy a missing-file exception or claim an old consumer proves new rules.
T145–T151 close the retained-code debts; T116/T134/T135 require the complete gate.
The immutable [T001 audit](../audit/README.md) is not rewritten to look current.

A C test registration is checked by including its actual translation unit in a
syntax-only probe, renaming its original `main`, and statically requiring every
registered case to have type `void (void)`. The probe uses the real leaf Makefile's
`COMPILE_COMMAND`, preserving fixture defines and include paths. This proves name
and signature, not assertion quality or execution. Review the independent expected
outcomes and run the registered commands before claiming behavioral evidence.
Ready ordinary headers compile as the first and only include, followed by a neutral
C11 static assertion so macro-only/marker headers do not make an empty translation
unit. Authored native payload fragments explicitly supply only their documented
SDK/namespace context through compiler options. A deliberate test-project #error
header instead requires exit code 1 and its exact configured diagnostic; unexpected
acceptance, a different error or a failed tool does not satisfy that contract.
Purpose checks parse
actual C documentation comments; markers hidden in literals do not count.

Every compiler Doxygen contract block also needs an explicit canonical rule link.
This uniform check covers function/type/helper comments without guessing which
operation is nontrivial from its size or spelling. A narrow implementation names
its relevant rule; a generic helper may cite the surrounding file's declared
component rules, with its existing purpose/invariant explanation. Links in code
examples, literals or another comment block cannot supply that contract.

## Exact source-to-HTML mapping

The registry reads requirement links from `spec/requirements.md`. A canonical
entry must resolve inside `spec/` to one existing explicit anchor immediately
followed, after whitespace, by that exact requirement's definition. Duplicate
identities/destinations, mismatched labels and stale links fail. No nearest rule,
filename guess or fallback destination is used.

C comment extraction implements trigraph replacement and escaped-newline removal
before recognizing strings, characters and comments, with offsets back into the
physical file. Adjacent Doxygen line comments share Markdown state. CommonMark
block/inline rules recognize actual links; wrappers retain offsets while nested
quote/list state exists. Table splitting follows the pinned parser's exact rule.
Fences, inline code, images, literal C strings and Doxygen code/verbatim examples
are not rewritten. An unsupported parser version or mapping disagreement fails
rather than guessing. Markdown input must use LF and contain no NUL.

An identity hashes a length-framed tuple of kind, full repository path and stable
rule ID with SHA-256. The registry stores the original tuple and rejects a hash
collision between distinct tuples. Rule definitions become `@anchor` on the same
line. Links become `@ref` only in disposable views under `build/documentation/`;
page identities are explicit, deterministic labels. The explicit narrative scope includes root guides and maintained Markdown under
`docs/`, `stdlib/` and `third_party/`, excluding generated trees. Links to these
non-authoritative pages map by complete resolved path to their own page identity;
they cannot satisfy a compiler file's mandatory canonical-rule link. Rule links incoming from
other specification pages use the same mapping. All transformed files preserve
line counts. Maintained input hashes are rechecked after generation.

Doxygen alone renders `docs/technical/`. The checker parses actual HTML and its
tag file, requires every mapped link on its originating page, verifies each rule's
unique definition belongs to its canonical chapter and checks the target file
and fragment. It also requires a file compound for every maintained code input.
`build/documentation/report.json` records versions, input fingerprints and counts.
Canonical-link and extracted-interface checks are strict. Doxygen emits XML as
well as HTML; the gate requires actual nonempty brief or detailed descriptions on
every extracted function, typedef, enum and class/struct/union. EXTRACT_ALL keeps
missing interfaces visible to that check rather than silently omitting them.
Fields, macros and enum values use the enclosing documented type contract.
An absent description, malformed/missing XML, warning or unresolved canonical
page/rule destination fails generation. Counts refer to extracted declaration
records, not a claim of unique external ABI symbols.

Skipped Markdown heading ranks are compacted only in temporary views. CommonMark
supplies real top-level heading tokens; fenced examples and nested containers are
untouched. A stack assigns each heading its nearest lower-ranked predecessor as
parent, then uses parent display rank plus one. This preserves the exact parent
tree, title text, order and title-derived anchors while removing Doxygen's invalid
rank jumps. It edits only ATX hash prefixes and preserves every source newline.
Specification files are never edited to accommodate the renderer.

## Invariants and argument

For every unmasked documentation character, the position map denotes its exact
physical source character. Block wrappers retain the parser's active offsets,
inline wrappers report only links in the root inline output, and reverse-order
nonoverlapping edits preserve all other bytes. Each replacement preserves its
span's newline count. Thus source line numbers remain invariant, and examples and
non-documentation bytes remain unchanged. The maintained files are never written.
The canonical map is a bijection between indexed IDs and file/anchor pairs;
checked unique generated identities and canonical chapter validation preserve it
through rendering. Tests exercise distinct boundary failures, not only a positive
string replacement. For N input bytes, E edits, R rules and H generated links, a conservative
upper bound is O(E N + R N + H R), with linear-sized source/HTML storage plus
registries. This documentation tool is not a runtime compiler path; Doxygen's
own cost is separate.

## Bootstrap evidence and source navigation

The bootstrap is `src/native/status/status.h`: ERROR-03 defines code/message
representation and ERROR-04 defines borrowed program-lifetime message storage.
Its existing native status leaf tests success and an unterminated borrowed
message; the header compiles independently under strict C11 flags. The generated
file page must link to both canonical rules, not merely have those IDs in text.

Supported source navigation was checked in the installed VS Code CLI 1.145.1:
`code --goto /home/u/repos/ama/spec/implementation/diagnostics.md:156` returned
zero and requested the exact ERROR-03 definition. The absolute path/line belong
only to this recorded local check; maintained C links remain ordinary relative
Markdown URLs with stable rule anchors. This evidence does not claim an automated
GUI mouse-click test. Recalculate paths after moves; the documentation fixture
checks a deep source, rejects its stale moved link, then renders the corrected
moved source successfully.


Maintained C includes use the `.inc` extension (for example the pinned Unicode
ranges). Doxygen must parse them with `EXTENSION_MAPPING = inc=C`; its default for
that suffix is PHP. T145's real-rendering regression includes a canonical rule link
on an `.inc` file page, so the generated file-page/link check detects losing this
mapping rather than merely counting an empty file compound.

T151 closes the remaining ready/debt partition: 675 maintained inputs have explicit
purpose, contract and named consumer evidence; the debt registry is empty. Inventory
retains expected observation files and the standard-library contract independently
of that empty history. This documents retained code; language conformance still
follows the unfinished roadmap. T134 verifies the full baseline, warnings, all extracted interfaces and two-run
byte reproducibility. T135 remains the final post-implementation regeneration.

The [T116 closure](closure.md) reconciles the expanded current inventory, actual
component contracts, standalone headers, named tests and rendered reference
after the feature/tool implementation tasks. The live ready registry remains
the per-file evidence map; checkpoint counts do not excuse subsequent omissions.
