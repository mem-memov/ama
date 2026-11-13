# Composed Ama and ABI parsing

This boundary completes the structural parser work in T022–T028. It connects
original UTF-8 source ownership, physical lines, token boundaries, sections,
declarations, sources, bindings, result trees and control frames. Parsing records
source forms under [the grammar](../../spec/grammar.md); it does not choose types,
infer declarations or execute modules.

## Flat ABI records use their own lexical context

[PROTO-11](../../spec/implementation/native-interface.md#proto-11) defines six
explicit slots. A `| function` line is an ABI binding, never an Ama literal with
raw payload. ABI layout still validates UTF-8, TAB and physical CR rules before
allocating lines. The flat-line check rejects spaces-only blank lines and trailing
code spaces without a comment. Comment lines may have arbitrary indentation.
Record parsing then requires column one and exactly one separator before one C
identifier. It neither trims repeated separators nor turns trailing spaces into
an implicit correction. C identifiers/UCNs retain their supported C lexical
contract independently of Ama multiword XID names.

Each marker maps to exactly one explicit slot; duplicate records retain both
original marker sites. Only the presence of `+` and `-` determines whether an
otherwise valid table has its required lifecycle records. Other capabilities
remain absent until explicitly bound. Names do not discover capabilities, and
different slots may use the same spelling at this syntactic stage. Typed native
signature compatibility belongs to the later C boundary.

For a finite table, physical splitting visits every byte once, record recognition
visits every code byte once, and the fixed six-slot lookup is bounded. The source
and line arenas own every successful/partial record view. A table is published
valid only after every physical record and mandatory slot succeeds.

## Examples are checked in their stated contexts

`tests/unit/parser/composition/support/documented.c` declares an explicit
chapter/fence/context manifest for 63 canonical Ama/ABI text fences. It reads the
current specification without editing it. Complete modules are parsed unchanged;
body/declaration fragments receive an explicit `@!` context. The constructor-only
and clone-only ABI illustrations receive the explicitly listed missing lifecycle
records. Filesystem trees, compiler dependency diagrams and directory decomposition
illustrations are excluded because they are not Ama programs. No lookup or
activation heuristic decides which example receives which wrapper.

Twenty-two additional composed forms exercise exact joins, Unicode names,
raw literal/hash/trailing bytes, both root families, bindings and placeholders,
self references, result trees, loops, traversals, handlers, comments and CRLF.
A generic test-side AST inspection checks every reachable structural subtree:
all spans borrow the admitted source, original bytes remain unchanged, linked
block endpoints agree, argument ordinals match independent direction counts,
selected operations retain their parent, and ordinary output actions alias the
corresponding bindings. Folded loop terminals resolve to the last written header
operation. This validates actual cross-component handoffs rather than only a
parse-success flag.

Every byte prefix of a composed Unicode program is checked, including partial
UTF-8, unfinished markers and complete prefixes. A prefix may be valid or invalid;
invalid input must retain a valid bounded diagnostic and never publish a partial
module. Every actual source-to-AST allocation is failed once, including source
admission, frames, actions and required warning records. All owners are destroyed
and the counted allocator verifies zero remaining resources.

## Finite ABI domain and failure checks

The ABI leaf independently enumerates every ordered subset of six slots:

`sum(k=0..6) 6!/(6-k)! = 1957`.

For each subset, its independent bit-set oracle accepts exactly when both required
slots occur. Each slot's duplicate markers are checked independently. Malformed
whitespace, missing/native identifiers, unknown markers, Unicode/UCNs, raw-pipe
near-matches and flat indentation have negative cases. Every byte truncation of
a mixed-ending Unicode ABI table and every composed source/ABI allocation failure
also preserve the table's explicit validity and ownership contracts.

Run `make -C tests/unit/parser`, or the independent `abi` and `composition`
leaves. Per-file named cases and commands live in `documentation/ready.json`.
Runtime support remains governed by later semantic, lowering and execution tasks;
structural acceptance of a canonical example does not claim those features run.
