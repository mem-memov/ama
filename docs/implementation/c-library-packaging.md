# Compiled component artifacts and code lifetime

This is implementation documentation for
[EMBED-06](../../spec/implementation/c-library.md#embed-06), not another language
specification. A component is one complete generated C translation unit with a
checked public boundary. Its Ama calls still use the single label dispatcher
described in [C-library execution](c-library-execution.md).

## Build and consume a component

```sh
./ama --library-build path/to/project path/to/component.exports \
    --standard-root stdlib/modules --out path/to/component-output --sdk src
cc -std=c11 host.c path/to/component-output/library.a -o host
```

The command builds both `library.a` and `library.so` for the supported GNU
x86-64 Linux ELF LP64 System V profile. It also publishes `public.h`,
`component.json`, `component.c`, `symbols.txt`, `native-model.json`, the private project assembly and its native
declarations. Include `public.h` from the C host; it needs no compiler SDK.
Do not compile all the published C files together: `component.c` already includes
`project.c`. No executable `launcher.c` is published.

The configuration and its ordered settings participate in the complete component
key. Put ABI-affecting compilation flags in `c-flag` records, rather than supplying
unrecorded flags through a library-build CLI option. Each record is one exact
argument, including embedded spaces. Required C11, diagnostics, non-LTO, PIC and
default-public-visibility flags follow the user flags. `link-file` and `link-flag`
records follow the isolated object in configuration order. The shared linker
rejects unresolved imports. Static consumers must supply the recorded external C
dependencies and flags when linking their final executable; the component archive
does not silently absorb unrelated dependency archives.

Builds use a private temporary directory. The compiler does not publish any
library until compilation, symbol verification, deterministic archiving and shared
linking have all succeeded. The existing owned-artifact publisher then commits
the complete output set. A failing rebuild preserves the previously usable set.
`--out` and `--sdk` are mandatory; `--cc` selects the explicit compiler program.
`--library-check` and `--library-header` remain read-only requests.

The [C host examples](c-library-examples.md) provide a complete CLI-driven
static/shared build with exact argument records and explicit C dependencies.
`symbols.txt` publishes the same exact symbol set used by the packaging verifier;
`native-model.json` maps generated native authoring identities and filenames.

## Symbol isolation is an exact set operation

The checked model determines the complete ordered set of public implementation
symbols: the ABI bootstrap, lifecycle/diagnostic/code-lifetime functions, each
schema's view/clone/release functions, and only the selected export operations.
There is no prefix-based inference about handwritten native contributions.

The compiler builds one unstripped non-LTO PIC object. GNU `objcopy` retains global
linkage only for that exact set; every other definition becomes local. Undefined
platform C imports keep their original spelling. This prevents both static-link
collisions and ELF interposition between components containing the same Ama
modules or native helpers. Visibility alone would not establish this property.

Before packaging, GNU `nm` evidence is checked twice:

1. The raw object must contain no registered native entry or Ama control-entry
   symbol, including a dot-delimited compiler specialization of such an entry.
2. The isolated object's externally visible definitions must equal the complete
   declared set, with each expected name appearing exactly once.

The first check reuses the backend's bounded listing parser and exact registered
entry-family matcher. The second compares full names, not hashes or prefixes.
A stripped/incomplete object, a missing public definition, an unexpected external
definition, a malformed listing or a failed tool prevents packaging. GNU `ar`
uses deterministic member metadata. Public header bootstrap checks still compare
the full component key and every declared layout fact before structure-based calls.

## Code ownership extends beyond a context

Each generated component has an independent code-dependency registry. Every
scalar/list snapshot registers an intrusive private owner before publication;
partial construction registers nothing that its cleanup cannot consume. Nested
list owners are retired in reverse construction order. This registry also allows
view/clone/release to check the exact live address and schema before dereferencing
a supplied opaque snapshot pointer. A foreign component's valid pointer does not
become a local owner just because its ABI has a compatible layout.

An immutable diagnostic acquires a separate code pin. Releasing its API result
or diagnostic consumes that pin and empties the public owner. A diagnostic may
therefore remain readable after context disposal, but still prevents code unload.
Copying the C structure does not create another releasing owner.

Detached native activity uses the generated component-qualified operations
`code_pin`, `code_unpin` and the initialized-empty `CodePin` owner. The pin has
one owner and a checked, nonwrapping local identity. Acquire it before publishing
any callback/completion that may require this image. A callback may update its
host completion record without executing Ama. Release the pin only after the
callback has returned and its executor/retirement protocol proves that no further
operation needs the image. Disposing the originating context does not release
such a detached pin automatically.

`code_can_unload` returns API OK only when there are no context records and no
snapshot, diagnostic or detached callback pins. Busy returns no diagnostic owner,
so testing readiness does not itself introduce a new dependency. An instance is
covered by its owning context; retained views are covered by their snapshot owner.
The registry's short atomic gate executes no allocation or native hook.

The supported loader must hold an **external call barrier**: exclude all new and
in-flight API calls, keep its system loader ticket through the readiness query's
return, and unload only after an OK response. A pin released from a function's
body cannot keep that same function's return instructions resident by itself.
The component cannot intercept an unrelated owner's forced `dlclose`.
Do not call any saved function pointer after unload; a reload must resolve a new
set of pointers and perform its ABI bootstrap again.

When several static Ama components reside in one shared image, the integration
must query **every** such component before unloading that image. A component's
query accounts for its own resources, not for another component's private
registry. Recorded external dependencies and the integration's explicit ownership
determine this group; inspecting archive filenames is not a discovery algorithm.

## Reloads cannot revive old numeric controls

A generated component binds its service to a fresh load-domain receipt before
publishing the first context. This receipt is an allocated byte whose address
is used only as an identity, never dereferenced through a public handle. A
published receipt is deliberately retained for process lifetime, even after the
library unloads. Live allocations cannot reuse its address, so the next image
cannot revive an old numeric context/instance control even if the operating
system reuses the image base and its local context counter starts again.

This is explicit retired identity history: one byte plus allocator overhead per
component image that attempts its first valid context creation. It has no code,
data-pointer or release-operation dependency on the unloaded image. It is not an
active snapshot/native-resource leak or an unload pin. Memory consumption is
O(number of such images), not O(number of calls or contexts). Losing unpublished
receipt candidates are freed. Allocation failure publishes no context and allows
a later factory retry. All reads/publication of the domain identity are atomic;
the service metadata gate selects the sole published candidate.

## Acceptance evidence

`make -C tests/unit/library/package` produces two independent components with
matching Ama/native text module names. Separate C host translation units consume
both public headers and test static linkage, ordinary shared linkage, and a
genuine `dlopen`/`dlsym`/`dlclose` path with repeated reloads. They exercise real
entry/yield/resume, stale controls, snapshots surviving context disposal,
independent clones, diagnostic pins and a late native completion pin. The loader
clears every function pointer after `dlclose`; it never probes dangling code.
Another C translation unit includes both public headers and rejects foreign
snapshot view/clone/release, context disposal and callback-pin release while
preserving the legitimate owners in both components.
Declared compiler flags reach the generated source; hidden user visibility cannot
hide the declared public boundary. Sanitizer builds instrument both the compiler
test and generated libraries/host, including an assertion-enabled component and
another compiled with `NDEBUG`. An injected first receipt-allocation failure
leaves the context empty, retains only its returned diagnostic, and permits a
successful factory retry after diagnostic release.
A separate injected callback-pin allocation failure leaves its destination empty
and introduces no unload dependency; retry succeeds without recreating a context.

The suite also rejects incompatible targets, foreign/missing assembly metadata,
input/output aliases, existing staging outputs, failed tools and an actual object
containing a deliberately surviving native entry specialization. Exact subprocess
argument bytes and signal/nonzero exits are checked separately.
Header include operands containing quotes, controls or C11 trigraphs are rejected
before writing any assembly; these bytes cannot inject another directive.

`make -C tests/integration/library-build` exercises the actual product CLI,
independent public-header-only C consumption, repeatable metadata, absence of an
executable launcher, and preservation of ready artifacts after a compiler failure.
`make -C tests/unit/runtime/library` checks exact owner membership, wrong schemas
and addresses, arbitrary unlink order, unregistered partial owners, independent
pins, counter exhaustion and underflow without wraparound.

The complete asynchronous retirement protocol and isolated concurrent component
compositions remain T099/T106 work. These artifact tests do not claim a general
dynamic plugin discovery service or automatic callback cancellation.
