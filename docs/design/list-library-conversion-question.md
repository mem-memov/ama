# Open question: converting list interface values

Status: deferred for discussion. Neither option below is an accepted language or
ABI requirement. This note does not amend the technical specification.

## Question

How should the compiler/runtime decode a typed list action and construct typed
length, completion and outcome results without assuming a module's native payload
representation?

## Why a decision is needed

The technical specification defines a fixed four-input/three-output list interface
in LIST-04 and six actions in LIST-08. Length/index and condition values belong to
library module types; actions and outcomes are nominal enumeration values.
LIB-01 forbids guessing their numeric payloads and requires validated identities
and signatures for compiler mappings.

The existing native `[]` protocol converts a value to a C `size_t`; `?` converts a
value to a C `bool`. Neither specifies the reverse conversion. The `|` protocol
constructs a typed value from literal bytes, but the interpretation of those bytes
belongs to its library module. Its existence alone does not establish decimal
encoding, boolean spelling or enumeration tags. Likewise, an enumeration does not
automatically provide a decoder for list actions.

Configuring the four library type identities therefore does not by itself specify
all conversions required to execute the whole-list interface.

## Option A: a library profile using existing ABI protocols

Require an explicitly documented profile for the configured library types:

- Decode actions through `[]`: Inspect=0, Append=1, Insert=2, Remove=3,
  Clear=4, Swap=5.
- Construct length through `|` from an unsigned decimal byte sequence.
- Construct completion through `|` from `0` or `1`.
- Construct outcomes through `|` from documented codes. The published library
  currently defines success=0, empty=7, stale=8, busy=9 and bounds=12.

This adds no syntax or ABI symbols and leaves native payload layout opaque.
However, it introduces an encoding obligation for configured library types;
existing read capabilities alone do not satisfy that obligation.

Before adopting this option, specify capability validation, supported value ranges,
conversion failures and how retained-result reads copy stored typed results.
Reconstructing a result through `|` must not silently replace the required copy
semantics or introduce unspecified observable constructor effects.

## Option B: explicitly registered C adapters

Register typed adapters for decoding actions and constructing length, completion
and outcome values. Bind them through validated module identities and signatures,
without discovering helper names or inspecting payload layout.

This separates runtime conversion from literal syntax and permits other literal
encodings. It also requires a concrete registration mechanism, signatures,
lifetime rules and error handling. None of those additions is approved yet.

## Work remaining after the decision

Update the technical specification only after an explicit decision, then align
ABI documentation and implementation. Complete whole-list entry and retained-read
integration, including positional skips, defaults, typed result copying and the
compiled-project acceptance tests in section 10 of TODO.md.

Ordered handle storage, standalone ownership-safe mutations and published scalar
library modules are already implemented. Their existence does not establish that
either conversion option has been accepted. Implementation work is paused at the
user's request after committing this note.
