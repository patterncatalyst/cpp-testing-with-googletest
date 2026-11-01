# Conan 2 recipe — the template every example copies.
# GoogleTest + GoogleMock only: these are testing examples, not services, so
# there is no logging stack or app dependency here. Pin exactly; never a range.
#
#   conan install . --output-folder=build --build=missing -s build_type=Debug
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class ExampleConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        # GoogleTest 1.18 pulls Abseil (and RE2); Conan pins them transitively,
        # so there is nothing to manage by hand.
        self.requires("gtest/1.18.0")

    def generate(self):
        # No cmake_layout: with `conan install . -of build`, the toolchain and
        # dep configs land flat in build/, which is where CMakePresets.json's
        # toolchainFile (build/conan_toolchain.cmake) looks for them.
        CMakeToolchain(self, generator="Ninja").generate()
        CMakeDeps(self).generate()
