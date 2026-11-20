# Module source catalog

The boundary is [catalog.h](../../src/source/module/catalog.h), with package
selection, module scanning, exact-entry probing and immutable source ownership.
It implements canonical source identity under
[SRC-01..05](../../spec/language/source-and-names.md#src-01) and whole-version
selection under [PKG-01/02](../../spec/project/packages.md#pkg-01). Dependency
closure, parsed activation, type resolution and configuration interpretation
remain separate operations. [T231's integration record](versioned-source.md)
identifies tested handoffs and unfinished acceptance; specification approval is
not implementation evidence.

## Roots, versions and module areas

A build borrows a finite list of explicitly tagged roots and exact root-relative
physical exclusions. `MODULE_ROOT_PROJECT` identifies a project root containing
optional `packages/` and `dependencies/` trees. `MODULE_ROOT_STANDARD` identifies
a standard-library module tree directly. The caller supplies the kind; the
root's physical basename never determines it.

The selector completes every local-package pass before considering dependencies.
Its key is the full vendor/package/version identity, including any variant.
A local copy wins for that complete version; an ignored external copy supplies
no modules, metadata or fallback files. Two copies at the same priority fail.
Different versions remain distinct. Selection neither parses configurations nor
chooses a highest version. Version fields are canonical decimal strings without
host-integer conversion or a numeric width cap.

Only the selected ordinary version's required `src/` and `tests/` areas are
indexed. Its `package.ini` must be a regular file; both areas must be directories.
Their logical prefixes retain vendor, package, version and area, for example
`/acme/shop/v2.4.14/src/item`. Configuration contents and source path
normalization are separate consumers. Standard modules use identities such as
`//integer`, without an ordinary package wrapper.

Within an area, complete-name directories group modules. A `name+` directory
requires an exact regular sibling module named `name`; only the structural plus
is omitted from logical identity. Parent source bytes do not determine a child's
identity, and discovery does not import or execute the parent. For example,
`src/item+/validate` maps to the selected version's `/src/item/validate` suffix.
Physical source, companion and diagnostic paths retain the plus.

Regular extensionless files with valid complete names are candidates. Companion
extensions, hidden names and other noncandidate entries do not become modules;
malformed UTF-8 names are rejected. Exclusions select exact physical subtrees,
never textual prefixes: `output` does not exclude `output next`. They do not
change a version choice or permit an external copy to fill a local hole. There
are no guessed output names or reserved ordinary identifiers such as `build`.

## Views, ownership and failure

`module_path_valid` validates complete canonical absolute endpoints through the
bounded path component. It does not expand aliases or normalize relative forms.
Raw source spelling is recognized separately; see
[lexical tokens](lexical-tokens.md). Lookup compares the entire canonical name,
without a shorter-prefix search or a standard/project fallback.

Physical discovery verifies exact root components and entry spellings, rejects
symlinks, and registers source/directory device-inode identities. Duplicate
selected logical names and physical aliases fail before catalog publication.
Equal basenames in different namespaces remain distinct. POSIX inode values are
observations for alias checking, not portable logical identities. Source storage
must remain stable during a build; this is not an atomic filesystem snapshot.

The same package-selection driver also accepts an explicit synchronous reader
for a projected relocation view. The reader provides root metadata, exact child
metadata and complete sorted entries. It borrows its context through selection;
returned names and failures must outlive their consumer. The physical and
projected readers share selection and checked directory-identity insertion,
preventing different precedence or alias algorithms. The relocation preview
retains its projected catalog independently until destruction.

The catalog arena owns paths, selections, entries and pending work; its source
manager owns immutable bytes. Root descriptors are borrowed only during build.
Every failed prefix remains destructible, and retained error paths outlive the
caller strings that initiated the build. Lookup is enabled only after complete
success. A failure code needs no additional allocation. Missing, unsupported,
ambiguous, orphan, alias, I/O and allocation failures remain distinguishable.

Native companions append `.c`, `.h` and `.abi` to the physical module path.
Discovery does not select them. After required parsing establishes `@C`, native
selection validates present companions and publishes a complete immutable view.
Repeated successful selection reuses that view without allocation. Missing
companions remain available to explicit authoring; strict native admission checks
required capabilities later. Drafts, ordinary modules and unused sources do not
acquire stale native inputs. See the [closure contract](project-dependencies.md#native-activation-and-invalidation).

## Correctness and checks

Finite FIFO work replaces recursive traversal. Each queued namespace retains its
exact physical, relative and logical prefixes. Version boundaries stop namespace
traversal; selected source areas then append slash-separated complete names.
Directory identity is registered before enqueueing, so aliases cannot create a
revisit cycle. Full source-name and inode checks preserve injectivity before
publication. Two complete priority passes establish precedence independently of
root order; sorted entry lists eliminate filesystem enumeration order as a choice.

The [versioned-source record](versioned-source.md) states the selection complexity
and its independent finite grammar/model checks. Module enumeration costs
O(sum e_dir log e_dir) for sorting, plus exact parent-entry probing and linear
registry comparisons; worst-case uniqueness work is quadratic in registered
source/directory counts. Owned path and source storage is proportional to retained
bytes. No guessed depth limit or heuristic repair is used.

Named acceptance commands are `make -C tests/unit/source/path`,
`make -C tests/unit/source/module/packages`, and
`make -C tests/unit/source/module`, followed by the real project, identity,
native and relocation consumers recorded for T231. They cover Unicode/plus
storage, required selected inputs, exclusions, both priority/root orders,
physical aliases, reader callback failures and measured allocation/I/O refusal.
Their exact run configurations and limits belong to the task record; a catalog
pass alone does not establish complete language or code-generation conformance.
