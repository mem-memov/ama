# Requirement index

[Specification index](README.md)

Each requirement is defined once in its canonical chapter. This index is
navigation, not a second statement of behavior. Start with the chapter if
you need the model, examples, or reasons behind a rule.

## Language purpose and core

| Requirement | Chapter section |
|---|---|
| [CORE-01](introduction.md#core-01) | Why the core stays small |
| [CORE-02](introduction.md#core-02) | Why the core stays small |
| [CORE-03](introduction.md#core-03) | Why the core stays small |
| [CORE-04](introduction.md#core-04) | Native foundations and their limits |
| [CORE-05](introduction.md#core-05) | Native foundations and their limits |

## Source identity

| Requirement | Chapter section |
|---|---|
| [SRC-01](language/source-and-names.md#src-01) | Files and module paths |
| [SRC-02](language/source-and-names.md#src-02) | Resolving a path without shadowing |
| [SRC-03](language/source-and-names.md#src-03) | Complete names, including names with spaces |
| [SRC-04](language/source-and-names.md#src-04) | A type path does not create an instance |
| [SRC-05](language/source-and-names.md#src-05) | Keeping a module and its submodules together |

## Source layout and syntax

| Requirement | Chapter section |
|---|---|
| [SYN-01](language/source-and-names.md#syn-01) | Indentation and block boundaries |
| [SYN-02](language/source-and-names.md#syn-02) | Comments and blank lines |
| [SYN-03](language/source-and-names.md#syn-03) | Module sections and unfinished files |
| [SYN-04](language/source-and-names.md#syn-04) | Drafts and activation |
| [SYN-05](language/source-and-names.md#syn-05) | Drafts and activation |
| [SYN-06](language/source-and-names.md#syn-06) | Exact source spelling |

## Names and visibility

| Requirement | Chapter section |
|---|---|
| [NAME-01](implementation/compiler.md#name-01) | Generate names from complete identities |
| [NAME-02](implementation/compiler.md#name-02) | Detect hash collisions exactly |
| [NAME-03](implementation/compiler.md#name-03) | Preserve source mappings |
| [NAME-04](language/declarations.md#name-04) | One base-name namespace per module |
| [NAME-05](implementation/native-interface.md#name-05) | Qualify handwritten native symbols |
| [NAME-06](language/declarations.md#name-06) | Lexical visibility and no shadowing |

## Declarations and types

| Requirement | Chapter section |
|---|---|
| [TYPE-01](language/declarations.md#type-01) | Names, types, and storage forms |
| [TYPE-02](language/declarations.md#type-02) | Initial contents and construction order |
| [TYPE-03](language/declarations.md#type-03) | Data representations belong to modules |
| [TYPE-04](language/declarations.md#type-04) | Creating owned state without running it |
| [TYPE-05](language/declarations.md#type-05) | Exact compatibility between storage forms |
| [TYPE-06](language/declarations.md#type-06) | Declaring a field at its first receiver |

## Static module templates

| Requirement | Chapter section |
|---|---|
| [TPL-01](language/templates.md#tpl-01) | Placeholder spelling |
| [TPL-02](language/templates.md#tpl-02) | Parameters and their scope |
| [TPL-03](language/templates.md#tpl-03) | Inferring types from typed connections |
| [TPL-04](language/templates.md#tpl-04) | Nested template instances |
| [TPL-05](language/templates.md#tpl-05) | Specialization identity and independent state |
| [TPL-06](language/templates.md#tpl-06) | Checking a specialization before execution |
| [TPL-07](language/templates.md#tpl-07) | Type parameters and template parameters |
| [TPL-08](language/templates.md#tpl-08) | Partial substitution and composition |
| [TPL-09](language/templates.md#tpl-09) | Templates and Ama's own lists |
| [TPL-10](language/templates.md#tpl-10) | Compiler obligations and design gates |

## Literal data

| Requirement | Chapter section |
|---|---|
| [LIT-01](language/declarations.md#lit-01) | The bytes after a vertical bar |
| [LIT-02](language/declarations.md#lit-02) | Literal interchange format |
| [LIT-03](language/declarations.md#lit-03) | A literal gets its interpretation from its destination |
| [LIT-04](language/declarations.md#lit-04) | Validation and construction failures |

## State and interfaces

| Requirement | Chapter section |
|---|---|
| [STATE-01](language/declarations.md#state-01) | Permanent body fields |
| [STATE-02](language/declarations.md#state-02) | Parameters and retained results |
| [STATE-03](language/declarations.md#state-03) | Shared immutable storage |

## Shared constants

| Requirement | Chapter section |
|---|---|
| [CONST-01](language/declarations.md#const-01) | What may be constant |
| [CONST-02](language/declarations.md#const-02) | Construction and cleanup order |
| [CONST-03](language/declarations.md#const-03) | Reading another module’s constants |
| [CONST-04](language/declarations.md#const-04) | Named enumeration values |
| [CONST-05](language/declarations.md#const-05) | Using a qualified constant directly |

## References and lifetime

| Requirement | Chapter section |
|---|---|
| [REF-01](language/values-and-references.md#ref-01) | Named references and self-reference |
| [REF-02](language/values-and-references.md#ref-02) | What a checked handle identifies |
| [REF-03](language/values-and-references.md#ref-03) | Destination first, then source |
| [REF-04](language/values-and-references.md#ref-04) | Liveness and generations |
| [REF-05](language/values-and-references.md#ref-05) | Generation exhaustion never wraps |
| [REF-06](language/values-and-references.md#ref-06) | Reclaiming blocks must retain identity history |
| [REF-07](language/values-and-references.md#ref-07) | Forming a handle does not acquire its target |
| [REF-08](language/values-and-references.md#ref-08) | Preflight all owned state before destruction |

## Copying values

| Requirement | Chapter section |
|---|---|
| [COPY-01](language/values-and-references.md#copy-01) | Independent ownership and preserved external references |
| [COPY-02](language/values-and-references.md#copy-02) | Preparation, commit, and failure |
| [COPY-03](language/values-and-references.md#copy-03) | Value copying clears saved execution positions |
| [COPY-04](language/values-and-references.md#copy-04) | The ownership boundary determines reference remapping |
| [COPY-05](language/values-and-references.md#copy-05) | An exact source-to-destination identity map |
| [COPY-06](language/values-and-references.md#copy-06) | Capability checks, self-assignment, and overlap |
| [COPY-07](language/values-and-references.md#copy-07) | The complete source tree must be available |

## Calls and results

| Requirement | Chapter section |
|---|---|
| [CALL-01](language/calls-and-results.md#call-01) | Entry requires an explicit exclamation mark |
| [CALL-02](language/calls-and-results.md#call-02) | Inputs, defaults, and complete value results |
| [CALL-03](language/calls-and-results.md#call-03) | Reading retained results without executing the target |
| [CALL-04](language/calls-and-results.md#call-04) | Refusing acquisition or a documented list preflight |
| [CALL-05](language/calls-and-results.md#call-05) | A local alternative with a precise failure boundary |
| [CALL-06](language/calls-and-results.md#call-06) | Leaving positional inputs or outputs unused |
| [CALL-07](language/calls-and-results.md#call-07) | Constructing an input from literal bytes |
| [CALL-08](language/calls-and-results.md#call-08) | The exact order of a call |
| [CALL-09](language/calls-and-results.md#call-09) | Calling a result directly |

## Control flow and traversal

| Requirement | Chapter section |
|---|---|
| [FLOW-01](language/control-flow.md#flow-01) | Choosing a branch |
| [FLOW-02](language/control-flow.md#flow-02) | Repeating a body |
| [FLOW-03](language/control-flow.md#flow-03) | Which values can be conditions or indices? |
| [FLOW-04](language/control-flow.md#flow-04) | Computing a loop condition through calls |
| [FLOW-05](language/lists.md#flow-05) | Traversing a changing list |

## Continuations

| Requirement | Chapter section |
|---|---|
| [CONT-01](language/continuations.md#cont-01) | Return to the immediate caller |
| [CONT-02](language/continuations.md#cont-02) | Store everything needed to resume explicitly |
| [CONT-03](language/continuations.md#cont-03) | Resume preserves the interface and accepts no new inputs |
| [CONT-04](language/continuations.md#cont-04) | Copy data, owned descendants, and saved control state |
| [CONT-05](language/continuations.md#cont-05) | Availability and resource-clone requirements |
| [CONT-06](language/continuations.md#cont-06) | Yield only at an Ama statement boundary |

## Lists

| Requirement | Chapter section |
|---|---|
| [LIST-01](language/lists.md#list-01) | Owning lists and lists of references |
| [LIST-02](language/lists.md#list-02) | Selecting an element by its current position |
| [LIST-03](language/lists.md#list-03) | Required representation and complexity |
| [LIST-04](language/lists.md#list-04) | Calling list methods |
| [LIST-05](language/lists.md#list-05) | Costs and stable identity |
| [LIST-06](language/lists.md#list-06) | Removing owned values and reference entries |
| [LIST-07](language/lists.md#list-07) | A queue is a separate collection workload |
| [LIST-08](language/lists.md#list-08) | What each method changes |
| [LIST-09](language/lists.md#list-09) | Where new entries get their values |
| [LIST-10](language/lists.md#list-10) | Access protection and safe failure |
| [LIST-11](language/lists.md#list-11) | Length and whole-list copies |
| [LIST-12](language/lists.md#list-12) | Examples: adding, removing, and observing entries |
| [LIST-13](language/lists.md#list-13) | Membership identity and traversal bookkeeping |

## Asynchronous tasks

| Requirement | Chapter section |
|---|---|
| [ASYNC-01](language/async-and-parallelism.md#async-01) | Start with the existing continuation rules |
| [ASYNC-02](language/async-and-parallelism.md#async-02) | One executor, several tasks |
| [ASYNC-03](language/async-and-parallelism.md#async-03) | A task reports what should happen next |
| [ASYNC-04](language/async-and-parallelism.md#async-04) | Keep the scheduling relationships visible |
| [ASYNC-05](language/async-and-parallelism.md#async-05) | Bridge native events without leaking Ama references |
| [ASYNC-06](language/async-and-parallelism.md#async-06) | Bridge native events without leaking Ama references |
| [ASYNC-07](language/async-and-parallelism.md#async-07) | Queue discipline and progress |
| [ASYNC-08](language/async-and-parallelism.md#async-08) | Cancellation, completion, and resource lifetime |
| [ASYNC-09](language/async-and-parallelism.md#async-09) | Cancellation, completion, and resource lifetime |
| [ASYNC-10](language/async-and-parallelism.md#async-10) | Continuation copying and failures |
| [ASYNC-11](language/async-and-parallelism.md#async-11) | Continuation copying and failures |
| [ASYNC-12](language/async-and-parallelism.md#async-12) | Implementing an asynchronous host |

## Parallel contexts

| Requirement | Chapter section |
|---|---|
| [PAR-01](language/async-and-parallelism.md#par-01) | Parallelism uses isolated execution contexts |
| [PAR-02](language/async-and-parallelism.md#par-02) | Parallelism uses isolated execution contexts |
| [PAR-03](language/async-and-parallelism.md#par-03) | Messages cross the boundary; references do not |
| [PAR-04](language/async-and-parallelism.md#par-04) | Messages cross the boundary; references do not |
| [PAR-05](language/async-and-parallelism.md#par-05) | Native concurrency and failure boundaries |
| [PAR-06](language/async-and-parallelism.md#par-06) | Native concurrency and failure boundaries |
| [PAR-07](language/async-and-parallelism.md#par-07) | Implementing and verifying parallel contexts |
| [PAR-08](language/async-and-parallelism.md#par-08) | Implementing and verifying parallel contexts |

## Runtime

| Requirement | Chapter section |
|---|---|
| [RT-01](implementation/runtime.md#rt-01) | Pools belong to an execution context |
| [RT-02](implementation/runtime.md#rt-02) | Keep state and control metadata in stable slots |
| [RT-03](implementation/runtime.md#rt-03) | Protect storage while it is in use |
| [RT-04](implementation/runtime.md#rt-04) | Availability belongs to each instance |
| [RT-05](implementation/runtime.md#rt-05) | Execute directly over instance storage |
| [RT-06](implementation/runtime.md#rt-06) | Enter a nested invocation |
| [RT-07](implementation/runtime.md#rt-07) | Finish a body and process its results |
| [RT-08](implementation/runtime.md#rt-08) | Record explicit return links |
| [RT-09](implementation/runtime.md#rt-09) | Separate return links from saved positions |
| [RT-10](implementation/runtime.md#rt-10) | Clean up every exit path |
| [RT-11](implementation/runtime.md#rt-11) | Create one explicit root |
| [RT-12](implementation/runtime.md#rt-12) | Reuse free slots without reviving identities |
| [RT-13](implementation/runtime.md#rt-13) | Reclaim entirely free blocks without moving live storage |
| [RT-14](implementation/runtime.md#rt-14) | Return an explicit outcome to the host |
| [RT-15](implementation/runtime.md#rt-15) | Resume or dispose a yielded root |
| [RT-16](implementation/runtime.md#rt-16) | Keep launcher policy explicit |

## Native interfaces

| Requirement | Chapter section |
|---|---|
| [CABI-01](implementation/native-interface.md#cabi-01) | Connect the declared interface to C |
| [CABI-02](implementation/native-interface.md#cabi-02) | Preserve nominal type identity |
| [CABI-03](implementation/native-interface.md#cabi-03) | Pass typed values and bounded borrows |
| [CABI-04](implementation/native-interface.md#cabi-04) | Describe native resource representation |
| [CABI-05](implementation/native-interface.md#cabi-05) | Keep Ama references out of opaque payloads |
| [CABI-06](implementation/native-interface.md#cabi-06) | Generate a usable native authoring interface |

## Compiled libraries and C embedding

| Requirement | Chapter section |
|---|---|
| [EMBED-01](implementation/c-library.md#embed-01) | Build an explicitly exported component |
| [EMBED-02](implementation/c-library.md#embed-02) | Own contexts and exported instances explicitly |
| [EMBED-03](implementation/c-library.md#embed-03) | Enter, resume, and read through a checked C boundary |
| [EMBED-04](implementation/c-library.md#embed-04) | Transfer independently owned typed data |
| [EMBED-05](implementation/c-library.md#embed-05) | Keep application and execution failures distinct |
| [EMBED-06](implementation/c-library.md#embed-06) | Isolate names, ABI editions, allocators, and code lifetimes |
| [EMBED-07](implementation/c-library.md#embed-07) | Own a foreign component as a native resource |
| [EMBED-08](implementation/c-library.md#embed-08) | Complete nested C boundaries before yielding |

## Native protocols

| Requirement | Chapter section |
|---|---|
| [PROTO-01](implementation/native-interface.md#proto-01) | Declare the six fixed native protocols |
| [PROTO-02](implementation/native-interface.md#proto-02) | Read explicit bindings, not C naming conventions |
| [PROTO-03](implementation/native-interface.md#proto-03) | Use checked native transport signatures |
| [PROTO-04](implementation/native-interface.md#proto-04) | Conversions inspect current state |
| [PROTO-05](implementation/native-interface.md#proto-05) | Reject invalid indices distinctly |
| [PROTO-06](implementation/native-interface.md#proto-06) | Clone native data independently |
| [PROTO-07](implementation/native-interface.md#proto-07) | Derive copy capability over owned types |
| [PROTO-08](implementation/native-interface.md#proto-08) | Select one fresh constructor |
| [PROTO-09](implementation/native-interface.md#proto-09) | Report technical status explicitly |
| [PROTO-10](implementation/native-interface.md#proto-10) | Continue cleanup after destruction errors |
| [PROTO-11](implementation/native-interface.md#proto-11) | Read the native binding table exactly |

## Lifecycle

| Requirement | Chapter section |
|---|---|
| [LIFE-01](implementation/native-interface.md#life-01) | Track fresh, constructing, live, and finalized storage |
| [LIFE-02](implementation/native-interface.md#life-02) | Construct fields in declaration order |
| [LIFE-03](implementation/native-interface.md#life-03) | Destroy each resource once |
| [LIFE-04](implementation/native-interface.md#life-04) | Prepare replacements before destructive commit |
| [LIFE-05](implementation/native-interface.md#life-05) | Dispose a failed execution consistently |

## Refusals and failures

| Requirement | Chapter section |
|---|---|
| [ERROR-01](implementation/diagnostics.md#error-01) | Return expected failures as ordinary results |
| [ERROR-02](implementation/diagnostics.md#error-02) | End this context on technical failure |
| [ERROR-03](implementation/diagnostics.md#error-03) | Use one explicit status representation |
| [ERROR-04](implementation/diagnostics.md#error-04) | Keep error messages valid without allocation |
| [ERROR-05](implementation/diagnostics.md#error-05) | Separate invalid-index refusal from conversion failure |
| [ERROR-06](implementation/diagnostics.md#error-06) | Remember the caller's last refusal |

## Execution diagnostics

| Requirement | Chapter section |
|---|---|
| [DEBUG-01](implementation/diagnostics.md#debug-01) | Map execution to source locations |
| [DEBUG-02](implementation/diagnostics.md#debug-02) | Capture the active path before cleanup |
| [DEBUG-03](implementation/diagnostics.md#debug-03) | Describe active calls, not execution history |
| [DEBUG-04](implementation/diagnostics.md#debug-04) | Return from native C before reporting the Ama path |

## Generated C

| Requirement | Chapter section |
|---|---|
| [GEN-01](implementation/compiler.md#gen-01) | Emit one assembly per independently compiled component |
| [GEN-02](implementation/compiler.md#gen-02) | Lower Ama bodies to labels |
| [GEN-03](implementation/compiler.md#gen-03) | Store values that survive control transfers explicitly |
| [GEN-04](implementation/compiler.md#gen-04) | Emit from resolved IR |

## Stackless execution

| Requirement | Chapter section |
|---|---|
| [STACK-01](implementation/compiler.md#stack-01) | Avoid C activations for Ama calls |
| [STACK-02](implementation/compiler.md#stack-02) | Allow ordinary native helper calls |
| [STACK-03](implementation/compiler.md#stack-03) | Never yield across an unfinished native call |
| [STACK-04](implementation/compiler.md#stack-04) | Verify the generated control-flow property |

## Compiler stages

| Requirement | Chapter section |
|---|---|
| [COMP-01](implementation/compiler.md#comp-01) | Parse before resolving names |
| [COMP-02](implementation/compiler.md#comp-02) | Separate AST from typed IR |
| [COMP-03](implementation/compiler.md#comp-03) | Separate kinds of dependency |
| [COMP-04](implementation/compiler.md#comp-04) | Report precise failures and respect output ownership |
| [COMP-05](implementation/compiler.md#comp-05) | Generate deterministic results |

## Dependency resolution

| Requirement | Chapter section |
|---|---|
| [DEP-01](implementation/compiler.md#dep-01) | Resolve dependencies through configured roots |
| [DEP-02](implementation/compiler.md#dep-02) | Build one exact module registry |
| [DEP-03](implementation/compiler.md#dep-03) | Track the complete dependency closure |

## Compiler architecture

| Requirement | Chapter section |
|---|---|
| [ARCH-01](implementation/compiler.md#arch-01) | Decompose the compiler by responsibility |
| [ARCH-02](implementation/compiler.md#arch-02) | Keep each operation at one abstraction level |
| [ARCH-03](implementation/compiler.md#arch-03) | Expose narrow component contracts |
| [ARCH-04](implementation/compiler.md#arch-04) | Test components and their compositions |
| [ARCH-05](implementation/compiler.md#arch-05) | Use exact algorithms |
| [ARCH-06](implementation/compiler.md#arch-06) | Explain the general correctness argument |

## Library contracts

| Requirement | Chapter section |
|---|---|
| [LIB-01](language/library.md#lib-01) | Validated native foundations |
| [LIB-02](language/library.md#lib-02) | Validated native foundations |

## Testing

| Requirement | Chapter section |
|---|---|
| [TEST-01](implementation/tools-and-conformance.md#test-01) | Use the project test harness |
| [TEST-02](implementation/tools-and-conformance.md#test-02) | Make each leaf executable explicit |
| [TEST-03](implementation/tools-and-conformance.md#test-03) | Connect recursive test subtrees |
| [TEST-04](implementation/tools-and-conformance.md#test-04) | Keep integration inputs and expectations together |
| [TEST-05](implementation/tools-and-conformance.md#test-05) | Exercise resource and failure paths |
| [TEST-06](implementation/tools-and-conformance.md#test-06) | Mirror the compiler responsibility tree |
| [TEST-07](implementation/tools-and-conformance.md#test-07) | Test real component compositions |
| [TEST-08](implementation/tools-and-conformance.md#test-08) | Derive checks from laws and invariants |

## Tools

| Requirement | Chapter section |
|---|---|
| [TOOL-01](implementation/tools-and-conformance.md#tool-01) | Move a source directory through resolved identities |
| [TOOL-02](implementation/tools-and-conformance.md#tool-02) | Compute the complete identity mapping |
| [TOOL-03](implementation/tools-and-conformance.md#tool-03) | Preserve bytes outside rewritten paths |
| [TOOL-04](implementation/tools-and-conformance.md#tool-04) | Publish the move as a recoverable transaction |
