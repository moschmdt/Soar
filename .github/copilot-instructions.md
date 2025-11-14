# GitHub Copilot Instructions for Soar CMake Project

## Project Overview
This is the Soar cognitive architecture project using CMake build system with Conan package management. The project supports multiple SWIG language bindings including Python and Java.

## Build System Rules
**ALWAYS use VS Code CMake extension commands for build operations. NEVER use direct terminal cmake commands.**

### Required CMake Extension Commands

#### Configuration Commands
- Use `cmake.configure` - Configure the project with current settings
- Use `cmake.selectConfigurePreset` - Select from available presets (Debug-swig-java, Debug-swig-both, etc.)
- Use `cmake.clean` - Clean build artifacts
- Use `cmake.editCacheUI` - Modify CMake variables through UI

#### Build Commands  
- Use `cmake.build` - Build the project
- Use `cmake.selectBuildPreset` - Select build preset matching configure preset
- Use `cmake.buildTarget` - Build specific target if needed

#### Management Commands
- Use `cmake.install` - Install built artifacts
- Use `cmake.ctest` - Run tests if BUILD_TESTS is enabled

### Project-Specific Configuration

#### Available CMake Presets
- `Debug-test` - Debug with tests enabled
- `Debug-swig-python` - Debug with Python SWIG bindings
- `Debug-swig-java` - Debug with Java SWIG bindings  
- `Debug-swig-both` - Debug with both Python and Java SWIG bindings
- `Release-test` - Release configuration with tests

#### Key CMake Options
- `SWIG_JAVA` - Enable/disable Java SWIG bindings (default: ON)
- `SWIG_PYTHON` - Enable/disable Python SWIG bindings (default: ON)
- `CLI` - Build Soar command line interface (default: OFF)
- `BUILD_TESTS` - Build unit and performance tests (default: OFF)
- `SVS` - Enable Spatial Visual System (default: OFF)
- `ASAN` - Enable AddressSanitizer in debug builds (default: OFF)

#### Project Structure
- Core libraries: `soar_lib` (main Soar library)
- SWIG bindings: Located in `Core/ClientSMLSWIG/`
- CLI application: `SoarCLI/`
- Tests: `UnitTests/` and `PerformanceTests/`
- Install directory: `install/` (configurable via CMAKE_INSTALL_PREFIX)

### Workflow Guidelines

#### For Java SWIG Development
1. Run `cmake.selectConfigurePreset` → choose "Debug-swig-java" or "Debug-swig-both"
2. Run `cmake.configure` 
3. Run `cmake.selectBuildPreset` → choose matching build preset
4. Run `cmake.build`

#### For Clean Rebuild
1. Run `cmake.clean`
2. Run `cmake.configure`
3. Run `cmake.build`

#### For Testing
1. Ensure BUILD_TESTS is ON via `cmake.editCacheUI`
2. Run `cmake.configure`
3. Run `cmake.build`
4. Run `cmake.ctest`

### Troubleshooting

#### Common Issues
- **Conan preset conflicts**: Always use CMake extension commands, not direct cmake terminal commands
- **Missing SWIG dependencies**: Ensure SWIG, Java JDK, Python development headers are installed
- **Java binding compilation errors**: Check that `JavaCallbackByHand.h` is properly copied to build directory

#### File System Constraints
- Project may be in read-only container - use CMake extension commands only
- VS Code tasks.json creation may be restricted - rely on CMake extension UI

### Integration Points
- **Conan**: Package dependencies managed via ConanPresets.json
- **SWIG**: Language bindings for Python and Java
- **SQLite**: Required dependency for Soar kernel
- **VS Code**: Primary development environment with CMake extension

## Code Generation Guidelines
When generating code for this project:
- Include proper CMake targets in CMakeLists.txt files
- Ensure SWIG interface files (.i) are properly configured  
- Follow existing code organization in Core/ directory structure
- Include appropriate install() commands for new components

## Testing and Validation
- Java SWIG bindings generate .java files and native library (libsml.so/sml.dll)
- Python bindings create soar_sml module
- CLI builds to soar executable
- Install process populates install/ directory with libraries, headers, and bindings

Always verify builds complete successfully with chosen preset before proceeding with development tasks.