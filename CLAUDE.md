# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is Soar

Soar 9.6.x is a cognitive architecture for building intelligent agents. The C++ kernel implements the decision cycle, working memory, production matching (RETE), and long-term memory subsystems. External programs interact via the SML (Soar Markup Language) API — either in-process (embedded) or over a socket (remote, XML-framed).

## Build Systems

Two parallel build systems exist; CMake is the current focus for new development.

### CMake (preferred)

Install Conan dependencies first (one-time or after `conanfile.py` changes):

```shell
pip install conan
conan profile detect --force
conan install . --build=missing
conan install . --build=missing -s build_type=Debug
```

Then build + install:

```shell
cmake --workflow --preset Debug-static   # configure + build + test
cmake --install build/Debug              # installs to ./install/
```

Useful presets (see `CMakePresets.json`): `Debug-static`, `Debug-shared`, `Release-static`, `Release-shared`, `Debug-test-asan`, `Debug-swig`.

Generate `compile_commands.json` for IDE IntelliSense: it is generated automatically by CMake (`CMAKE_EXPORT_COMPILE_COMMANDS=ON`).

### SCons (legacy, still used for Windows, Tcl, C# bindings)

```shell
# Debug build (activates assertions)
python3 scons/scons.py --scu --dbg --verbose all

# Optimized build
python3 scons/scons.py --opt --verbose all

# Generate compile_commands.json only
python3 scons/scons.py --scu --opt --verbose cdb
```

## Running Tests

### CMake — CTest

```shell
# All tests via preset
ctest --preset Debug-static --output-on-failure

# Single named test
ctest --preset Debug-static -R test_operator_explanation_gtest --output-on-failure
```

### Custom test harness (test_soar binary)

The `test_soar` binary (built by `UnitTests/CMakeLists.txt`) has its own runner:

```shell
build/Debug/UnitTests/test_soar --list-categories
build/Debug/UnitTests/test_soar --list-tests
build/Debug/UnitTests/test_soar -c BasicTests          # run one category
build/Debug/UnitTests/test_soar -t myTestName          # run one test
build/Debug/UnitTests/test_soar -E SvsTests            # exclude category
build/Debug/UnitTests/test_soar -e PRIMS_Sanity1       # exclude test
```

### GTest targets (tests/ subdirectory)

```shell
build/Debug/tests/test_operator_explanation_gtest
build/Debug/tests/test_identifier_exceptions_gtest
```

### Integration test

```shell
./integration.sh   # builds Release-shared, installs, runs downstream CMake consumer
```

## Code Style

- **Allman style**, 4-space indent, no tabs, Linux line endings.
- Pointer/reference operators attached to type: `Symbol* sym`, `Agent& agent`.
- Underscores in names (`my_function`, `my_variable`).
- All filenames lowercase.
- New namespaced code goes under `soar::`.
- Pre-commit hooks enforce: trailing whitespace, LF endings, no large files, clang-format (`.clang-format` style file), and Conventional Commits format for commit messages.

Run `pre-commit run --all-files` before pushing, or install hooks with `pre-commit install`.

## Architecture

### Layer stack (top → bottom)

```
SoarCLI / Java Debugger / Python scripts
    ↓
ClientSML  (Core/ClientSML/)       — public C++ API for embedding or remote clients
    ↓ XML-over-socket or direct call
KernelSML  (Core/KernelSML/)       — bridges SML protocol to kernel operations
    ↓
CLI        (Core/CLI/)             — command parser, one file per command group
    ↓
SoarKernel (Core/SoarKernel/)      — decision cycle, RETE, WM, learning subsystems
```

Supporting layers: `Core/ConnectionSML/` (socket transport), `Core/ElementXML/` (XML DOM), `Core/shared/` (portability, memory pools, SQLite wrapper, output manager).

### SoarKernel subsystems (`Core/SoarKernel/src/`)

| Directory | Content |
|---|---|
| `decision_process/` | Decision cycle: `decide.cpp`, RETE (`rete.cpp`), consistency checks |
| `soar_representation/` | Core data structures: `agent`, `symbol`, `production`, `preference`, `condition`, `instantiation` |
| `episodic_memory/` | EpMem (episodic memory) implementation |
| `semantic_memory/` | SMem (semantic memory) + SQLite storage |
| `explanation_based_chunking/` | Chunking / EBC learning |
| `explanation_memory/` | Explainability/trace subsystem |
| `operator_explanation/` | `OperatorExplanationManager` — tracks why operators were selected, SQLite-backed |
| `reinforcement_learning/` | RL module |
| `interface/` | Kernel-level interface wiring |
| `output_manager/` | `printa_sf()` and trace output (custom format engine, not printf) |
| `parsing/` | Soar rule parser / lexer |
| `shared/` | Constants, enums, memory manager, `soar_db` SQLite helpers |
| `debug_code/` | Debug-only instrumentation |

### SML remote protocol

Remote clients connect via TCP. Each message is framed as `uint32 big-endian length` + raw XML bytes. The authoritative set of supported command names is the `m_CommandMap` built in `KernelSML::BuildCommandMap()` (`Core/KernelSML/src/sml_KernelSMLHandlers.cpp`). A machine-readable catalog is at `docs/sml-xml-command-catalog.json`.

Launch kernel in listen mode: `install/bin/soar -l -p 12121` (the `-l` flag, not `-n` which toggles color). Probe with `scripts/sml_socket_probe.py`.

### Key invariants

- `printa_sf()` uses its own format specifiers: `%s`=`char*`, `%y`=`Symbol*`, `%d`=`int64_t`, `%u`=`uint64_t`, `%f`=`double`, `%l`=`condition*`, `%t`=`test`, `%a`=`action*`. Never use `%lld`, `%ld`, `%p`, etc.
- `soar_module::sqlite_statement::bind_text()` uses `SQLITE_STATIC` — the string must outlive the statement execute. Use `bind_text_transient()` for local/short-lived strings; it guards against `nullptr` by calling `sqlite3_bind_null`.
- Soar preference syntax: `>` best, `<` worst, `=` unary-indifferent, `!` require, `-` reject, `~` prohibit. The word `best` is not a preference specifier; it creates a symbolic constant.
- Impasse type constants (`constants.h`): `NONE=0`, `CONSTRAINT_FAILURE=1`, `CONFLICT=2`, `TIE=3`, `NO_CHANGE=4`.
- `SoarCLI -c <file>` is syntax-check only; `-s <file>` sources and executes.
- `UnitTests/SoarUnitTests/` is glob-included into `test_soar`; gtest-only sources must be excluded from the glob and built in a dedicated target under `tests/`.

## Dependencies

Runtime (managed by Conan for CMake): `SQLite3`, `spdlog`, `nlohmann_json`. SWIG is optional for language bindings (Python, Java, JavaScript). SVS (Spatial Visual System) is disabled by default (`NO_SVS` define).
