# AGENTS.md — Living Codebase Memory (Soar)

## Purpose

This file is the persistent memory for coding agents working in this repository.

It must be kept up to date whenever an agent learns something crucial about the codebase.

## Non-Negotiable Rule

On every task, if you discover important new information, update this file in the same change set before finishing.

Important new information includes (not exhaustive):

- Build, test, or runtime commands that actually work (or fail in a specific way)
- Project structure and ownership boundaries
- Architectural constraints and invariants
- Naming, style, or API conventions expected by maintainers
- Platform-specific behavior (macOS/Linux/Windows differences)
- Debugging workflows, useful scripts, and known pitfalls
- Any “gotcha” that could cause future regressions or wasted time

If nothing new was learned, state that explicitly in your final handoff.

## Update Protocol (Mandatory)

1. Read this file at the start of the task.
2. As you work, collect durable facts (not temporary speculation).
3. Before finishing, append or revise the relevant section(s).
4. Add a short entry to **Learning Log** with date and what changed.
5. Keep entries concise and factual.

## Confidence Labels

Use one of these tags for key facts:

- **[confirmed]** verified by running tools/commands or direct code inspection
- **[inferred]** strongly implied by code structure but not directly executed
- **[needs-validation]** likely true, but not yet validated

## Repository Snapshot (Initial)

- **Repo name:** Soar
- **Top-level build systems present:** CMake and SCons [confirmed]
- **Notable top-level dirs:** `Core/`, `SoarCLI/`, `UnitTests/`, `PerformanceTests/`, `Java/`, `Tcl/`, `scripts/`, `build/` [confirmed]
- **Existing build artifacts directory:** `build/` with `Debug/` and `Release/` subtrees [confirmed]

## Architecture Notes

Populate with stable architectural facts. Keep this focused on cross-cutting structure.

- [confirmed] `Core/` contains primary subsystem implementations and shared components.
- [confirmed] `SoarCLI/` contains CLI-specific sources and build scripts.
- [confirmed] `UnitTests/` contains test sources and build definitions.
- [confirmed] `Kernel::CreateRemoteConnection(...)` in `Core/ClientSML/src/sml_ClientKernel.*` uses `Connection::CreateRemoteConnection(...)` to connect over sockets to an already-running kernel process.
- [confirmed] `RemoteConnection::SendMsg`/`ReceiveMessages` in `Core/ConnectionSML/src/sml_RemoteConnection.cpp` serialize SML messages as XML strings (`GenerateXMLString`, `SendString`, `ReceiveString`, `ParseXMLFromString`).
- [confirmed] `DataSender::SendString`/`ReceiveString` in `Core/ConnectionSML/src/sock_DataSender.cpp` frame each XML message as `4-byte network-order length` + `raw XML bytes`.
- [confirmed] The authoritative set of client->kernel XML query commands is the `m_CommandMap` initialized in `KernelSML::BuildCommandMap()` at `Core/KernelSML/src/sml_KernelSMLHandlers.cpp`.

## Build & Test Knowledge

Record only commands validated in this repository context.

### CMake

- [confirmed] A configured build tree exists under `build/Debug/`.
- [needs-validation] Preferred VS Code workflow for CMake tasks should use CMake Tools integration for build/test execution.
- [confirmed] Top-level `CMakeLists.txt` builds a unified `soar_lib` from `ClientSML`, `ConnectionSML`, `ElementXML`, `KernelSML`, and `SoarKernel` sources.
- [confirmed] CI workflow `.github/workflows/cmake-multi-platform.yml` selects CMake presets by matrix as `${build_type}-${shared|test}`; current matrix hits `Debug-test`, `Debug-shared`, `Release-test`, and `Release-shared`.
- [confirmed] `BUILD_PERFORMANCE_TESTS` must be explicitly set per preset to keep performance tests out of CI, because top-level `CMakeLists.txt` adds `PerformanceTests/` when the variable is undefined.
- [confirmed] CPack package contents are driven by install rules: executable install coverage now includes `soar` (`SoarCLI/CMakeLists.txt`), `test_soar`/`test_external_lib` (`UnitTests/CMakeLists.txt`), and `PerformanceTests` when that target is built (`PerformanceTests/CMakeLists.txt`).
- [confirmed] GitHub Actions workflow `.github/workflows/cmake-multi-platform.yml` now gates release publishing via a `release-gate` job and publishes CPack `.tar.gz`/`.deb` artifacts to GitHub Releases on `release` events and on tag pushes whose commit is an ancestor of the repository default branch.

### Protocol probing

- [confirmed] A standalone raw-socket probe script exists at `scripts/sml_socket_probe.py` that tests remote protocol connectivity using commands `version` and `get_agent_list`, with optional `cmdline`.
- [confirmed] On macOS, running `install/bin/soar -l -p 12121` allows successful raw-socket probe execution (`python3 scripts/sml_socket_probe.py --host 127.0.0.1 --port 12121`) with valid `version` and `get_agent_list` responses.
- [confirmed] Remote `cmdline` calls via probe were validated against agent `soar` for commands `step`, `print <s>`, and `run 1`, returning expected raw CLI text responses.
- [confirmed] Machine-readable protocol artifacts exist at `docs/sml-xml-command-catalog.schema.json` and `docs/sml-xml-command-catalog.json`, derived from `KernelSML::BuildCommandMap()` and handler argument inspection.

### SCons

- [confirmed] SCons scripts are present (`SConstruct`, per-module `SConscript` files).

## Conventions & Guardrails

- Keep changes minimal and scoped to the user request.
- Fix root causes when feasible; avoid cosmetic churn.
- Do not change unrelated files opportunistically.
- Preserve existing style in each touched component.

## Known Pitfalls

Add concrete pitfalls here as they are discovered.

- [needs-validation] Multiple build systems (CMake + SCons) may diverge in generated outputs/configuration.
- [confirmed] The API supports both embedded and remote modes; using `CreateKernelInNewThread/CreateKernelInCurrentThread` embeds kernel code in-process, while `CreateRemoteConnection` communicates via socket/XML to an external kernel process. Confusing these modes can lead to incorrect deployment assumptions.
- [confirmed] Remote SML socket transport uses length-prefixed XML frames (not newline-delimited XML); adapters must implement exact framing to avoid desynchronization.
- [confirmed] `SoarCLI` flags are easy to misuse: `-n` disables ANSI color (it does not set port), and without `-l` the kernel runs in current-thread mode where remote requests may not be serviced while idle, causing socket client timeouts.
- [confirmed] In CI release jobs, `cpack --preset Release-test|Release-shared` can include `UnitTests` binaries because they are now installed; `PerformanceTests` binaries are still excluded in CI because CI-used configure presets set `BUILD_PERFORMANCE_TESTS=OFF`.
- [confirmed] `UnitTests/CMakeLists.txt` globs all `SoarUnitTests/*.cpp` into `test_soar`; any gtest-only source in that folder must be explicitly removed from `TEST_SOURCES` or `test_soar` will compile it without gtest include/link settings.

## Open Questions

Track unresolved but important uncertainties.

- Which build system is canonical for CI and release artifacts?
- What is the preferred full local test command for maintainers?

## Learning Log

Append one bullet per task when new crucial knowledge is learned.

- 2026-03-03: Initialized AGENTS memory file with mandatory update protocol and baseline repository facts. [confirmed]
- 2026-03-03: Confirmed SML remote mode uses XML-over-socket transport and that deployment requirements differ between embedded (`CreateKernelIn*`) and remote (`CreateRemoteConnection`) client modes; added CMake unified-library note. [confirmed]
- 2026-03-03: Confirmed SML socket framing detail for remote mode (`uint32` big-endian length prefix before XML payload), relevant for custom non-SWIG clients. [confirmed]
- 2026-03-03: Added protocol analysis doc at `scons/scons-local-4.4.0/SCons/Tool/docbook/docs/sml-xml-remote-protocol.md` and a runnable external probe script `scripts/sml_socket_probe.py` for non-library remote testing. [confirmed]
- 2026-03-03: Validated probe timeout cause and fix for `SoarCLI`: launch with `-l -p <port>`; `-n` is color toggle, not port argument. Verified successful probe response on `127.0.0.1:12121`. [confirmed]
- 2026-03-03: Fixed probe `get_agent_list` parsing to read `<name>` character data (now returns `agents=['soar']`) and validated remote `cmdline` execution for `step`, `print <s>`, and `run 1`. [confirmed]
- 2026-03-03: Confirmed `KernelSML::BuildCommandMap()` is the protocol source of truth for supported client->kernel SML `command` names (more reliable than constant declarations alone). [confirmed]
- 2026-03-03: Added machine-readable SML command catalog schema + instance under `docs/` to support downstream agent/tool generation workflows. [confirmed]
- 2026-03-10: Updated CI-used CMake presets (`Debug-test`, `Debug-shared`, `Release-test`, `Release-shared`) to explicitly set `BUILD_PERFORMANCE_TESTS=OFF`; added install rules for `test_soar`, `test_external_lib`, and `PerformanceTests` so CPack can include those executables when built. [confirmed]
- 2026-03-10: While adding `FindByAttribute` miss logging, confirmed this workspace does not currently expose `spdlog/spdlog.h` to `Core/ClientSML/src/sml_ClientIdentifier.cpp`; implemented conditional `__has_include` guard for spdlog-based logging with safe fallback. [confirmed]
- 2026-03-10: Fixed gtest build break by excluding `UnitTests/SoarUnitTests/IdentifierExceptionTests.cpp` from `test_soar` glob sources and building it only via dedicated `test_identifier_exceptions_gtest` target; verified by CMake build + passing CTest `test_identifier_exceptions_gtest`. [confirmed]
- 2026-03-10: After removing `FindByAttribute` throw, validated gtest log assertion pattern by routing default spdlog logger to `ostream_sink_mt` in `tests/UnitTests/IdentifierExceptionTests.cpp` and checking message text; verified with CMake build + passing CTest `test_identifier_exceptions_gtest`. [confirmed]
- 2026-03-10: Updated CI workflow to publish CPack artifacts to GitHub Releases for `release` events and for tag pushes on default-branch ancestry, using `release-gate` + `publish-release` jobs in `.github/workflows/cmake-multi-platform.yml`. [confirmed]
