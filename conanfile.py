# This file defines the cmake toolchain with the required dependencies to build
# Soar. The Python version is used instead of the pure text version since the
# user presets file must be changed.
#
# Conan documentation: https://docs.conan.io/2/

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os


class soarRecipe(ConanFile):
    name = "soar"
    version = "9.6.4"
    package_type = "library"  # Changed from "application" to "library" to export
    
    # Optional metadata
    license = "BSD-3-Clause"
    author = "Soar Group"
    url = "https://github.com/SoarGroup/Soar"
    description = "Soar cognitive architecture"
    topics = ("cognitive-architecture", "ai", "sml")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    
    # Options for the package
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }
    
    # Sources are in the same repository
    exports_sources = "CMakeLists.txt", "Core/*", "SoarCLI/*", "QtDebugger/*", "Config.cmake.in", "soarversion", "LICENSE.md"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("sqlite3/3.50.4")
        # Boost 1.85.0 is the last version with boost::process
        # Boost 1.86+ removed boost::process (it was experimental)
        self.requires("boost/1.85.0")
        # Qt removed - install system Qt for QtDebugger instead due to missing support
        # self.requires("qt/6.8.3")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        # The cmake user presets file must be changed from the default,
        # otherwise cyclic imports with toolchain file CMakePresets.json will
        # occur
        tc.user_presets_path = 'ConanPresets.json'
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        
        # Copy additional files if needed
        copy(self, "LICENSE.md", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        # Define the main library target - use the actual library name
        self.cpp_info.libs = ["soar-9.6.4"]
        
        # Set the CMake target name
        self.cpp_info.set_property("cmake_target_name", "soar::soar_lib")
        
        # Define include directories - headers are in include/soar
        self.cpp_info.includedirs = ["include", "include/soar"]
        
        # Ensure dependencies are propagated
        self.cpp_info.requires = ["sqlite3::sqlite3", "boost::headers", "boost::filesystem"]
