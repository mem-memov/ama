# Application arguments and one-shot launchers

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a command is a boundary adapter](#explanation-a-command-is-a-boundary-adapter)
- [Choose an entry and recognize named inputs](#choose-an-entry-and-recognize-named-inputs)
- [Construct text inputs and preserve defaults](#construct-text-inputs-and-preserve-defaults)
- [Read files and decode binary inputs explicitly](#read-files-and-decode-binary-inputs-explicitly)
- [Run once and return to the caller](#run-once-and-return-to-the-caller)

<!-- /chapter-contents -->

## Explanation: a command is a boundary adapter

A compiled application selects one concrete Ama module. Its generated launcher
turns process arguments into that module's inputs and starts one instance. The
argument values are runtime data: changing a value does not select another
specialization, change a CMake variable or require another executable.

The same argument adapter serves a normal application and a program included
inside a C application. The latter has a named C entry instead of its own
`main`, under [EMBED-09](c-library.md#embed-09). Each call runs a fresh program.
An integrator wanting retained instances or explicit resumption uses the
[stateful host interface](c-library.md#embed-02) instead.

This chapter specifies the adapter; [CMake application builds](application-builds.md)
specifies preparation and compilation. Neither chapter claims implementation
support without the corresponding acceptance evidence.

**Example: command invocation.** Assume the selected module declares the owned
input `message` with a literal-capable text Type, `input document` with a
literal-capable Type accepting the example file, and `image` with an explicitly
configured binary transport. The argument forms below illustrate those bindings;
they do not declare universal file or image Types:

```sh
ama-project run /acme/tools/v1.0.0/src/process --output /tmp/ama-process -- \
    --message='two words' \
    '--input document:text=message.txt' \
    --image:binary=image.bin
```

The shell removes its quotation syntax before the launcher sees the tokens.
The tool's `--` ends build options; the following three tokens belong to the
application. The source module does not receive quotation marks, the tool's
module selection or its output directory.

## Choose an entry and recognize named inputs

<a id="cli-01"></a>

**CLI-01.** An executable or source-inclusion generation request explicitly
selects one ordinary concrete executable root by its complete logical module
identity, subject to [EMBED-01](c-library.md#embed-01). An ordinary Ama body or
concrete native `@C` body is eligible. A static module, marker or open template
is not an executable root. A concrete ordinary facade can select a generic
implementation. Runtime strings never infer its missing Type parameters.

Generate a checked binding table from the root's `@<` declarations and the
explicitly configured host transports. Bindings refer to the exact declared
base name, excluding storage suffix notation. Preserve case, Unicode scalars,
single internal spaces, hyphens and underscores; do not normalize names, replace
spaces with hyphens, use basenames or add case-insensitive aliases.

The application accepts only these named token forms:

| Token | Meaning |
|---|---|
| `--name=value` | Supply the UTF-8 bytes of `value` as text under CLI-02 |
| `--name:text=path` | Read text bytes from the named file under CLI-03 |
| `--name:binary=path` | Read binary bytes through a configured decoder under CLI-03 |

Find the first ASCII `=` in one argument token. The nonempty portion following
`--` and preceding `=` identifies the input and optional transport suffix; the
whole remaining portion is the value or path, including any later `=`. The
colon suffix is exactly `:text` or `:binary`. Colon and equals are excluded
from Ama names by SRC-03, so neither split requires escaping or a heuristic.
An unknown suffix is an error. `--name=value` and `--name:text=path` address
the same input and cannot both occur in one invocation.

Preserve each token as supplied by the host. The adapter performs no additional
shell parsing, quote removal, interpolation, wildcard expansion, backslash
decoding or whitespace trimming. Source separators, comments and layout do not
apply to argument payloads. `--name=` supplies empty data. In file forms the
path must be nonempty; empty content is represented by an empty file.

Reject an unknown input, duplicate input, unsupported transport, missing `=`,
empty name or positional argument before executing the root body. There is
no implicit Boolean shorthand: `--enabled=1` supplies text, while `--enabled`
alone is malformed. This initial contract reserves no special application
flag such as `--help`; a declared input with that base name is ordinary.
The tool-level separator is not forwarded as an application argument.

Both `main` and the source-inclusion function use conventional C `argc`/`argv`
semantics. If present, `argv[0]` is an invocation label, not a module input.
Parse tokens `argv[1]` through `argv[argc-1]`. A direct C caller may supply
`argc == 0` and `argv == NULL` to request no bindings. Negative counts and
missing token pointers are invalid invocation shapes. For positive counts the
caller provides a readable array of at least `argc` nonnull, NUL-terminated
token strings; ordinary C pointer validity is the caller's responsibility.
The array also supplies a null sentinel at `argv[argc]`; the zero-count/null-array
case above is the sole exception. The adapter neither mutates nor retains the
array or its strings after return.
The invocation label may be used for presentation but never selects the root.

**Example: exact names and values.** For declared inputs `input name` and
`enabled`, these application tokens have the following meanings:

```sh
./program '--input name=a=b # c' --enabled=0
```

`input name` receives the bytes `a=b # c`. `enabled` receives `0`, interpreted
by its declared Type; the compiler does not introduce an untyped Boolean.

## Construct text inputs and preserve defaults

<a id="cli-02"></a>

**CLI-02.** Text binding requires an owned scalar input whose known native-backed
Type supports the `|` constructor under PROTO-01/PROTO-08. Validate host text
as UTF-8 and pass its immutable bytes with their exact length to that
constructor. The marker `|` is not added. A `|`, `#` or quote in the payload
is data, not Ama source syntax. Encoding conversion of platform argument strings
to this UTF-8 interface belongs to the documented target launcher; do not guess
the encoding or silently replace unrepresentable characters.

The declared Type interprets the payload under LIT-02 through LIT-04. Decimal
integer text is still text supplied to that Type; the launcher does not add
numeric inference, an enum registry or special Boolean conversions. Unsupported
reference, whole-list or compound text binding is diagnosed, rather than
constructing a guessed value. A concrete ordinary entry adapter can expose
text inputs and perform a richer conversion through ordinary Ama code.

An absent binding uses the input's ordinary declared initializer/default.
There is no invented requirement that every input appear on the command line,
no automatic requiredness based on its name, and no implicit empty-string
replacement. Explicit empty text still calls `|`; it differs from omission.
Defaulted complex inputs and unconsumed results need no external codec merely
because they belong to the selected root.

Integrate supplied values into [RT-11](runtime.md#rt-11)'s combined startup.
Choose supplied construction or ordinary default construction for each input,
once, in declaration order under CALL-07/CALL-08 and STATE-02. Later defaults
observe the already installed earlier inputs. Then initialize results and the
remaining instance state in the specified order. Do not first construct a
supplied input's default payload, overwrite it, or reset the initial interface
a second time. Declaration order, not command-token order, governs constructors.

Complete syntactic/binding/transport admission before starting the root body.
Constructor-content rejection remains the technical failure specified by
LIT-04; it is not turned into a refusal, an automatic retry, or an ordinary
application failure. Clean every successfully constructed part if preparation
fails. Context constant initialization and root initialization preserve their
existing lifecycle ordering and failure rules.

## Read files and decode binary inputs explicitly

<a id="cli-03"></a>

**CLI-03.** File transports read a named input stream to its supported bounded
byte length, reporting open/read/allocation/size failures. Relative file paths
are physical filesystem paths relative to the process working directory, not
Ama module paths or aliases. They are not resolved through the module registry.
For a `run` operation the build tool preserves the caller's working directory
for the child application unless the caller explicitly selects another one.

The exact path `-` selects standard input. Admit at most one such consumer over
all file transports before reading it; there is no implicit splitting or replay
to other parameters. A physical file named `-` can be addressed as `./-`.
Consume the selected stream once; preserve all bytes through the end of input.
Multiple ordinary file bindings may identify the same path and are distinct
reads. The launcher does not promise a shared filesystem snapshot.
After complete syntactic/binding admission, perform file reads and their input
construction in root parameter declaration order. An earlier failure prevents
later reads and constructions. Close each stream opened by the adapter on both
success and failure. Standard input is borrowed and must not be closed; its
consumed bytes are not rolled back. Use byte-preserving input operations on
supported targets, including suppressing platform text-mode newline translation.
If an embedded host stream mode is temporarily changed, restore it before
return, including failure cleanup; diagnose an unsupported byte-preserving mode.

For `:text`, validate strict UTF-8 and apply CLI-02 to the complete contents.
Preserve empty content, final line endings, CR, LF, TAB, embedded U+0000 and an
initial U+FEFF exactly as data; do not strip a byte-order mark or append a
terminator to the constructor's payload. These are host data, outside Ama
source-file lexical restrictions. Invalid UTF-8 is diagnosed before invoking
that input's text constructor. Inline argument strings cannot carry an embedded
NUL; the text-file byte view can because its length is explicit.

For `:binary`, perform no UTF-8 validation, newline conversion, byte swapping
or interpretation by the text literal constructor. The generation request
must explicitly provide a checked binary transport for that exact input and
complete nominal Type under [EMBED-04](c-library.md#embed-04). The transport
identifies its byte format, decoding declarations/implementation inputs,
validation and size/range limits, fresh-value construction, ownership and
failure/cleanup contract. Its concrete metadata format is a documented backend
interface, as for the existing checked C export schemas; it is not a new Ama
section, seventh `.abi` protocol or runtime capability lookup.

Validate configured schemas and declarations before accepting a generated
application. An unconfigured binary binding is unsupported even if another
Type has the same C layout. Never infer a decoder from a filename, cast a
buffer to a native payload, dump/restore C structure padding, or recover Ama
references from encoded addresses. Structured/reference-bearing transports
are admitted only where the explicit checked host contract actually supports
them; otherwise a concrete facade or explicit ordinary conversion is needed.

Supply the decoder a bounded immutable view borrowed for the construction.
After successful construction, the Ama value owns independent state or a resource
whose explicit native contract acquired its ownership. No borrowed file buffer
or argument token escapes its permitted lifetime. Decoder failure follows the
existing checked transport and technical-failure contract, with input name
and source-file context. File loading supplies data; it executes no module body.

**Why files are explicit.** A text argument beginning with `@` is still text.
An argument naming an existing file is still text unless `:text` or `:binary`
was written. This prevents changes to the filesystem from changing how an
otherwise identical command is interpreted.

## Run once and return to the caller

<a id="cli-04"></a>

**CLI-04.** Both generated executable startup and the named one-shot source
entry run one freshly created isolated execution context and its explicitly
selected root. Calls do not share a hidden default instance, constant-lifecycle
state, previous results or saved continuation. Immutable generated code may be
shared subject to PAR-02's native reentrancy requirements.

The one-shot entry borrows the invocation arguments for its call, initializes
the context/constants and fresh root under RT-11 and CLI-02, executes from entry,
performs supported cleanup and returns an `int` status. It must not call process
exit, leave a live context for a later implicit call, or retain a pointer into
the caller's arguments. The generated `main` forwards its original `argc` and
`argv` to this entry and returns its status. The function's exact path-derived
name and source-inclusion packaging are defined once in
[EMBED-09](c-library.md#embed-09).

Apply [RT-16](runtime.md#rt-16): completion with successful cleanup returns zero;
invalid invocation, preparation/technical failure, cleanup failure and an
unhandled root yield report diagnostics and return a documented nonzero status.
Do not invent an implicit event loop or resume the yielded root. Dispose it
without executing its continuation. Initialization failure executes no root
body. Failure reporting preserves original module/input and host-stage context;
the diagnostics need not parse translated C-tool messages to choose behavior.
Supported targets document their nonzero status allocation; this rule does
not assign platform-independent numbers to each error category.

An ordinary result, including a true application failure value under ERROR-01,
does not automatically change process status. The generic launcher neither
prints nor serializes root results, selects a result named `failure`, or casts
one result to an exit code. A module can perform declared I/O; a C integrator
needing typed results or retained state uses the explicit host interface under
EMBED-02 through EMBED-05. Such application results remain distinct from a
technical launcher failure.

**Acceptance.** Test exact multiword/Unicode labels, first-equals splitting,
explicit empty versus omitted values, reordered tokens, duplicate/unknown
bindings, unsupported text/binary Types, UTF-8 boundary failures, literal
constructor failure and partial cleanup. Exercise binary embedded zeros, empty
and oversized inputs, file/stdin errors, one-stdin admission, unchanged newline,
TAB/BOM/NUL text-file data and exact borrowed-buffer lifetime. Count initial
constructors to prove no default-then-overwrite or second interface reset.
Run repeated source-entry calls, technical and cleanup failures, root yield
and an ordinary application failure result. Verify fresh state, no process
exit, no hidden resumption and all required native ABI/inlining checks.
