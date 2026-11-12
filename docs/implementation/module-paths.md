# Module source catalog

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/source/module/catalog.h`, with `scan/`, `resolve/`, exact-entry
probing and immutable sources. Implements SRC-01/02/03/05. This is source discovery
and lookup, not dependency closure, draft parsing or type resolution.

A build takes a finite ordered list of project/library roots with explicit
root-relative output exclusions. Sources are regular files whose complete names
are Unicode identifiers. Directories are identifier groups or `identifier+`
submodule containers. Other well-formed UTF-8 entry names are not source
candidates; in particular `.git`, extensions, C/header/ABI companions are not
modules. Invalid UTF-8 filenames are unsupported paths. Exact excluded files or
subtrees are skipped before candidate recognition. No legal identifier such as
`build` or `generated` is automatically reserved. Future driver configuration
must supply its actual output exclusions.

A structural directory requires an exact regular sibling file without its final
ASCII plus. Strip only that plus in logical identity. The parent file's text does
not decide the child's identity and is not implicitly imported or executed.
Native companions append `.c`, `.h`, `.abi` to the physical filename. Existing
companions must be regular files; source and ABI bytes receive strict UTF-8
validation. C/header contents remain native inputs. Mandatory ABI bindings and
required-draft diagnostics belong to later parsing/resolution tasks.

The POSIX implementation checks each configured-root prefix and candidate through
exact directory-entry spelling, rejects symlinks, and retains physical source paths.
The catalog checks `(device, inode)` for source and directory aliases, including
repeated roots and directory cycles/bind aliases, without claiming inode numbers
are portable logical identities. Conflicting logical names are rejected across
all roots. Lookup is enabled only after the entire build succeeds; no root can
win by search order. Different case/canonical Unicode spellings remain distinct
when the filesystem can represent them. Native encoding here is POSIX UTF-8;
other platform adapters are not certified. Inputs must not mutate during build.

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
view lifetime. The retained logical-path validator still accepts only the old
single-dot sequence of Unicode words. Complete multiword components, the separate
standard-library root and their identity encoding remain T016/T017; exact-byte
filesystem tests do not certify those unimplemented source forms. Native companion
lookup and invalid UTF-8 diagnostics retain their original loaded bytes even on
failure, until catalog destruction.
