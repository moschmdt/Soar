# Conan package for Soar SpawnDebugger test application
#
# This is a test application that depends on the Soar package

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class SpawnDebuggerTestRecipe(ConanFile):
    name = "spawn_debugger_test"
    version = "1.0"
    package_type = "application"
    
    # Optional metadata
    license = "BSD-3-Clause"
    description = "Test application for Soar's SpawnDebugger functionality"
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    
    # Sources are in the same directory
    exports_sources = "CMakeLists.txt", "SpawnDebugger.cpp"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        # Depend on the Soar package
        # Use the local Soar package (will be created via 'conan create' or 'conan export')
        self.requires("soar/9.6.4")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
