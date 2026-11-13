# Module source catalog

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/source/module/catalog.h`, with `scan/`, `resolve/`, exact-entry
probing and immutable sources. Implements SRC-01/02/03/05. This is source discovery
and lookup, not dependency closure, draft parsing or type resolution.

A build takes a finite ordered list of explicitly tagged roots with exact
root-relative output exclusions. `MODULE_ROOT_PROJECT` is the ordinary project/
import family selected by `.`; `MODULE_ROOT_STANDARD` is selected by `..`.
Zero/unknown kind is invalid. The root's physical name does not determine its
kind and is absent from the logical component sequence. Sources are regular
extensionless files whose complete names satisfy the pinned multiword Unicode
predicate. Directories are complete-name groups or `name+` submodule containers. Other well-formed UTF-8 entry names are not source
candidates; in particular `.git`, extensions, C/header/ABI companions are not
modules. Invalid UTF-8 filenames are unsupported paths. Exact excluded files or
subtrees are skipped before candidate recognition. No legal identifier such as
`build` or `generated` is automatically reserved. Future driver configuration
must supply its actual output exclusions.

A structural directory requires an exact regular sibling file without its final
ASCII plus. Strip only that plus in logical identity. The parent file's text does
not decide the child's identity and is not implicitly imported or executed.
Native companions append `.c`, `.h`, `.abi` to the physical filename. Catalog
construction loads module sources without selecting those companions. After
required parsing establishes `@C`, `module_catalog_load_native` probes that
catalog-owned record's companions. Present companions must be supported regular
files; ABI bytes receive strict UTF-8 validation. Complete companion views publish
atomically and repeated successful requests reuse them without allocation. Missing
companions remain null for explicit authoring; strict native checks enforce them
later. Ordinary/unused modules and drafts do not acquire stale native inputs.
See the [closure contract](project-dependencies.md#native-activation-and-invalidation).

The POSIX implementation checks each configured-root prefix and candidate through
exact directory-entry spelling, rejects symlinks, and retains physical source paths.
The catalog checks `(device, inode)` for source and directory aliases, including
repeated roots and directory cycles/bind aliases, without claiming inode numbers
are portable logical identities. Conflicting complete logical identities are rejected across roots of the same
kind. Equal component sequences in different kinds are distinct. A physical
module exposed under both kinds still fails the device/inode alias check. Lookup is enabled only after the entire build succeeds; no root can
win by search order. Different case/canonical Unicode spellings remain distinct
when the filesystem can represent them. Native encoding here is POSIX UTF-8;
other platform adapters are not certified. Inputs must remain stable through catalog construction and explicit companion selection.

Traversal uses an explicit FIFO, not recursive C calls. Each directory enumerates
then sorts names by bytewise strcmp, eliminating readdir order dependence.
Induction over processed directories shows each queued entry has the accumulated
logical prefix and exact physical path. Ancestor inode repetition fails before
queuing, so finite acyclic input terminates. Checking all existing source records
before insertion establishes both unique logical identity and source-inode
injectivity. Failed builds retain owned issue paths, related paths and any invalid
source/offset until destruction, but expose no usable partial catalog.

The arena owns strings, entries and work records; the source manager owns immutable
text. Destroying both also cleans every partial build. No allocation is needed to
store a failure code. Allocation/size refusal maps to MODULE_OUT_OF_MEMORY;
I/O, missing, ambiguous, orphan, alias and unsupported-path causes remain distinct.
The first observed error under configured root order and sorted FIFO traversal is
reported. Error selection does not imply accepting ambiguity in another order.

For e entries, d directories, m sources and b loaded source/ABI bytes, enumeration
sorting costs O(sum e_dir log e_dir). Exact POSIX probes additionally scan parent
entries, up to O(sum e_dir squared), and uniqueness checks cost O(m squared + d
squared). Copied paths and enumeration records use their total byte size; source
storage is O(b). This deliberately exact initial catalog has no heuristic fallback
or hidden recursive depth limit; faster indexes can preserve these same laws.

Evidence: `make -C tests/unit/source/module` passed C11/pedantic/Werror. It composes
real filesystem discovery, UTF-8, nested Chinese `name+/` paths, C/header/ABI
companions, parent drafts and explicit output exclusions. Negative cases cover
both root orders for duplicate names, hardlinks, repeated directory roots,
symlinks, missing anchors and exact malformed source/ABI offsets. Every allocation
in a successful composed catalog build is separately failed and all ownership is
released. Case and normalization-distinct physical files retain distinct lookups.


T145 documents each catalog scan/resolution/source-publication phase and internal
view lifetime. T016 completes the catalog’s root-kind/multiword path contract;
T017 supplies its generated-identity encoding; T022 supplies exact tokens; T023 connects declaration/type paths and T024 completes access sources.
Native companion
lookup and invalid UTF-8 diagnostics retain their original loaded bytes even on
failure, until catalog destruction.

## T016: absolute roots and complete names

`module_path_valid` accepts exactly `.` or `..` followed by nonempty complete-name
components separated by dots. It uses `unicode_name` on every already-delimited
component. Root-only paths, a third leading dot, empty components, plus markers,
slashes and edge/repeated/separator spaces fail. It neither trims input nor turns
`..` into a filesystem parent. A valid lookup is exact string equality of this
complete spelling; `.item`, `.item state` and `..item state` may coexist and no
shorter prefix is searched when the complete path is absent.

Scanning begins with an empty logical prefix for Project or one dot for Standard.
Each validated component appends dot plus its original bytes, producing exactly
the required root marker. Ordinary namespace directories keep every component.
A plus directory removes exactly its last structural plus after validating the
remaining complete name and verifying its exact regular sibling. This does not
make the parent an imported module, inherited state or executable default object.
The physical source, companions and diagnostics retain every directory's plus.

For example, when `generated/库 2` is configured Standard, a module
`generated/库 2/item state` is `..item state`: the root directory's name is not a
logical component and need not itself be a legal language name. Under an ordinary
root, `商店 工具/商品 状态+/读取 数据+/详情 项目` is
`.商店 工具.商品 状态.读取 数据.详情 项目`, with the required sibling module files at
each plus boundary. The same tree under a Standard root keeps `..` instead.

Root inventories are borrowed only for build; published records and discovered
paths belong to catalog owners. Source/native-companion byte lifetime and failure
publication rules are unchanged. A NULL nonempty inventory and an unknown kind
fail explicitly, rather than guessing Project. Existing CLI source/`--root`
entries are explicitly tagged Project/import; the current-spec CLI standard-root
configuration remains T068. Declaration/type AST spans now use T023; expanded access sources remain T024; generated root identity encoding is now provided by T017. Passing catalog cases does not certify
those later source/codegen stages.

The mapping is injective after checks: root marker determines kind; each dot
separates a component because dots are not XID name characters; each complete
component preserves its original bytes including single internal spaces. Equal
complete identities are rejected before source publication. Equal physical inode
identity under different logical spellings is also rejected, so no root order can
silently select a conflicting source. Missing lookup searches no other root family.
Physical discovery still uses sorted FIFO traversal and exact entry probes; no
runtime name heuristic or recursive filesystem walk is added.

The new source/module cases check both root orders for equal names in distinct
families, coexistence with shorter names, missing cross-family results, invalid
path spellings and unknown root tags. They compose nested Chinese multiword
plus paths, draft anchors and C/header/ABI companions in each family. Same-family
standard duplicates and cross-family hardlinks retain both conflicting physical
paths. Missing multiword anchors report their exact required filename. Every
allocation in a successful extended Standard catalog is separately failed;
lookup stays disabled and teardown releases all partial owners. Existing source,
parser, project, compiler and driver fixtures now initialize their root kind
explicitly; they preserve their previously tested ordinary-root domain.

T016 acceptance passes the full strict `-DNDEBUG -Werror` test tree, focused
catalog/options ASan/UBSan leaves (leak detection disabled), complete coverage
of 769 maintained inputs and all Markdown links. Warning-free Doxygen checks
597 maintained files, 2449 interfaces and all 288 canonical/page destinations;
two complete generations produce byte-identical output.

## T018 deterministic inventory and owned outputs

The inventory walks sorted original directory-entry bytes with a FIFO directory
queue and prepends completed sources. Consequently equal trees created in opposite
orders yield the same source sequence for the same root configuration. This
order does not select a conflicting source: duplicate logical names and physical
aliases still fail before successful catalog publication. Root order may change
inventory order or which conflict is reported first, while successful exact
lookups and closure discovery retain their meaning.

Exclusions match an explicit root-relative physical path exactly or through the
next slash. Thus excluding `output` excludes `output/nested`, while retaining
`output next` and `other/output`. The check precedes metadata/source inspection;
generated bytes inside an owned output subtree cannot become Ama modules or
trigger irrelevant UTF-8 errors. No conventional name is silently excluded.
Drivers supply their owned output paths; their complete configuration is T068.

[Inventory scenarios](../../tests/unit/source/module/support/enumeration.c) create
the same finite tree in opposite file orders and compare every published name.
They place malformed bytes in excluded outputs and verify adjacent legal names,
unrelated nested directories and unchanged lookup outputs on absence. Run
`make -C tests/unit/source/module`. Existing alias/duplicate cases and every
catalog allocation failure remain part of the same leaf. These checks compose
with the [T018 full-identity registry](project-dependencies.md), rather than adding
a second activation scanner.
