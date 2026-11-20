# Project tools and an empty project

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Why project operations have their own boundary](#why-project-operations-have-their-own-boundary)
- [A small command for starting a project](#a-small-command-for-starting-a-project)
- [Preparing and repeating an experiment](#preparing-and-repeating-an-experiment)

<!-- /chapter-contents -->

The compiler answers whether prepared Ama definitions form a valid program and
how to translate that program. Preparing a checkout, downloading a dependency,
moving sources and running experiments are separate jobs. Keeping them separate
makes a compiler invocation predictable and lets the semantic playground compose
the same small operations as a person working from a terminal.

This chapter specifies the target tools. It does not assert that the current
compiler or an existing command implements the new project model.

## Why project operations have their own boundary

<a id="project-01"></a>

**PROJECT-01.** Provide project operations through a separately executable
`ama-project` tool, with its own command handling, orchestration, tests and
documentation. Keep that implementation outside the compiler pipeline. The
tool may use narrowly defined read-only source, configuration and resolution
components shared with the compiler. Neither executable may depend on the
other executable's command dispatcher or reach into its private mutable state.
An implementation language or self-hosting requirement is not imposed by this
separation.

The compiler `ama` reads already available project definitions, selected package
configuration, alias companions and required native inputs. It applies
[package selection](../project/packages.md),
[source resolution](../language/source-and-names.md#src-06), type and lifecycle
checks, and emits only the explicitly requested compilation outputs. It MUST NOT
download or update dependencies, change repositories or source configuration,
create project scaffolding, run module tests or move source trees as an implicit
part of compilation. Missing local modules produce diagnostics identifying the
unresolved identity and the explicit preparation operation needed; compilation
does not silently repair its inputs through the network.

Project operations include explicit initialization, dependency acquisition,
source relocation and test execution. Their source boundaries separate command
parsing, planning, read-only validation, transport/process adapters, publication
and cleanup. A package operation MUST NOT become a collection of branches inside
the compiler driver. The compiler's supported C-toolchain invocation remains
part of explicit compilation and does not grant package-management effects.

The directory mover is owned by the project-tool boundary and follows
[TOOL-01 through TOOL-04](tools-and-conformance.md#directory-relocation-utility).
It uses the same parsed path/configuration structures and resolver as compilation
and acquisition; it must not implement a second interpretation of Ama paths.
Native authoring scaffolds, source inspection and editor adapters likewise use
explicit tool entry points and read-only compiler interfaces rather than implicit
compilation side effects.

**Explanation.** A network outage can prevent an explicit fetch. It must not
change the meaning of compiling an already prepared project. Separate commands
also allow a playground to prepare once and compare several compiler invocations
or algorithm variants against precisely the same inputs.

## A small command for starting a project

<a id="project-02"></a>

**PROJECT-02.** The command

```sh
ama-project init DESTINATION
```

creates an empty Ama project in a new directory or an existing empty directory.
The destination is an explicit filesystem argument, not an Ama module path.
The command writes these initial files and directory:

```text
DESTINATION/
├── README.md
├── AGENTS.md
└── packages/
    └── README.md
```

The root README explains the project purpose, the optional `packages/` and
`dependencies/` storage roots, and how to add or acquire the first versioned
package. The packages README illustrates a vendor/package/version tree containing
`package.ini`, `src/` and mirrored `tests/`. It explains that a project composed
only of acquired dependencies may omit `packages/`; this initial directory is
a useful default, not a semantic requirement. `dependencies/` and the temporary
`download/` staging area are created only when an operation actually needs them.

The generated AGENTS file gives concise project-specific guidance: navigate by
full module identity, respect one-space Ama layout, keep helpers and per-module
companions together, preserve `src`/`tests` dependency separation, retain and run
the relevant tests, and use explicit project commands for source mutations.
The guidance explains `.ini` aliases and single-dot static names and links the
generated local Markdown files to one another. Examples and documented commands
must match the generating tool's specification edition. Do not embed the
generator developer's absolute checkout paths or links to nonexistent generated
documentation.

Initialization creates no vendor identity, version, sample executable, fixed
entry module, artificial package manifest or dependency. Any ordinary executable
module at any package depth remains selectable as an entry through an explicit
compiler request. The hints show the form of that request without prescribing
a project-wide main module. An empty project is valid storage scaffolding;
compiling it without a selected available module reports the ordinary missing
entry diagnostic.

Validate the destination and complete output plan before publication. Reject a
nonempty destination, a conflicting path or an unsupported filesystem alias;
never overwrite or merge user files. Prepare generated files in operation-owned
staging and publish only the complete scaffold. A failure before publication
preserves an existing empty destination and leaves no partially initialized
project. Cleanup touches only the operation's staging; a cleanup failure reports
the retained path. Concurrent initializations cannot both publish into the same
destination. Initialization performs no network access, compiler invocation,
test execution or implicit creation of a Git repository.

**Example: a later first package, not files created by init.**

```text
packages/
└── laboratory/
    └── sorting/
        └── v1.0.0-memory/
            ├── package.ini
            ├── src/
            │   └── sort
            └── tests/
                └── sort/
                    └── empty input
```

The programmer chooses these names. The source module's identity is
`/laboratory/sorting/v1.0.0-memory/src/sort`, and its test is an independently
selectable ordinary microprogram.

## Preparing and repeating an experiment

<a id="project-03"></a>

**PROJECT-03.** Acquisition and test commands take explicit project/selection
inputs and report their plan and result. Initial acquisition seeds carry explicit
repository mappings supplied by the invoking client/command, or select a named
existing package configuration. A dependency-only project needs no invented root
manifest or local package merely to supply those inputs. A fetched version's own
configuration governs its outgoing dependencies and never redirects the request
that fetched it, under PKG-06. Acquisition implements
[PKG-01 through PKG-10](../project/packages.md) and never runs fetched Ama code,
native lifecycle callbacks or arbitrary repository hooks to discover metadata.
Test execution is a distinct requested operation on the prepared test modules.
The presence of tests in an acquired unit does not authorize their execution.

Each selected test is an ordinary independent microprogram with its own resolved
source/native dependency closure. Use the compiler's normal checks and execution
boundary. A test runner may orchestrate isolated invocations and aggregate their
results, but must not invent a runtime test-registration ABI, combine persistent
test states or give a test access to another execution context's mutable state.
Parallel jobs own separate output/state paths; duplicate or conflicting jobs
must not race on generated C or project inputs.

Keep the reproducibility record with each acquisition/experiment result. Record
the exact selected logical identities, resolved repository commits and source
content hashes, the copied module/companion/test membership, effective
configuration and alias inputs, specification/compiler/tool editions, requested
entry/test modules and command arguments, and observed outcomes. Native/toolchain
inputs must be identified to the extent supported by their declared contracts;
the separately deferred discovery of installed native libraries is not supplied
by guessing from headers or binaries. Store reports/logs in explicit caller-owned
output destinations, outside the source namespace. This chapter does not reserve
an additional mandatory project artifact directory.

Resolving a branch once makes a single acquisition coherent; it does not prevent
an upstream author from changing a version directory or deleting history. The
record identifies what was tested. Keeping the complete required code and tests
in the project's own `packages/` and repository preserves the experiment inputs
independently of later external changes, under PKG-10. Tests establish the scoped
behavioral evidence described by [CORE-06](../introduction.md#core-06); they do not
waive the compiler's mandatory safety checks or constitute a universal proof.

Acceptance must cover an offline prepared compilation, missing local sources,
no hidden download/update/test effects, shared resolver agreement across tools,
the init file set and guidance, new/existing-empty/conflicting destinations,
publication and cleanup failures, concurrent destination claims, selective
source-and-test acquisition, independently executable tests and provenance that
matches the actual inputs. Source mutation tests use the mover's preview,
preflight, recovery and graph-preservation obligations rather than text-search
substitution.
