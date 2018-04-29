import os

from conans import ConanFile, CMake, tools

class GsageConan(ConanFile):
    name = "gsage"
    version = "1.0"
    license = "MIT"
    url = "https://github.com/gsage/engine"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "with_ogre": ["1.9.0", "2.1.0", "disabled"],
        "with_input": ["nothing", "OIS"],
        "with_librocket": [True, False],
        "with_lua_version": ["luajit-2.0.5", "luajit-2.1.0", "lua-5.1"],
        "with_recast": [True, False],
    }
    default_options = (
        "shared=False",
        "with_input=nothing",
        "with_ogre=disabled",
        "with_librocket=False",
        "with_lua_version=luajit-2.0.5",
        "with_recast=True",
        "OGRE:with_boost=False",
    )
    generators = "cmake"
    requires = (
        ("msgpack/2.1.3@gsage/master",),
        ("SDL2/2.0.5@gsage/master",),
        ("gtest/1.8.0@lasote/stable",),
    )

    def source(self):
        self.run("git clone https://github.com/gsage/engine")

    def config_options(self):
        lua, version = str(self.options.with_lua_version).split("-")

        lua_package = "{}-rocks".format(lua)

        self.requires.add("{}/{}@gsage/master".format(lua_package, version), private=True)
        self.options[lua_package].shared = False

        self.options["gtest"].shared = False
        if self.settings.os == "Macos":
            self.options["SDL2"].x11_video = False

        if self.options.with_ogre == "1.9.0":
            self.requires.add("OGRE/1.9.0@gsage/master", private=True)
            self.options["OGRE"].shared = self.settings.os == "Windows"
            self.options["OGRE"].with_boost = self.settings.os != "Windows"
        if self.options.with_input == "OIS":
            self.requires.add("OIS/1.3@gsage/master", private=False)
        if self.options.with_librocket:
            self.requires.add("libRocket/1.3.0@gsage/master", private=False)
            self.options["libRocket"].with_lua_bindings = True
        if self.options.with_recast:
            self.requires.add("recast/latest@gsage/master")

    def build(self):
        cmake = CMake(self)
        options = {
            "CMAKE_INSTALL_PREFIX": "./sdk",
            "CMAKE_BUILD_TYPE": self.settings.build_type,
        }
        if self.settings.os == "Macos":
            options["CMAKE_OSX_ARCHITECTURES"] = "x86_64"

        if "luajit" in str(self.options.with_lua_version):
            options["WITH_LUAJIT"] = True

        if self.options.with_recast:
            options["WITH_RECAST"] = True

        if self.options.with_ogre != "disabled":
            options["OGRE_FOUND"] = True
            options["OGRE_STATIC"] = "NO" if self.options["OGRE"].shared else "YES"
            add_postfix = self.settings.build_type == "Debug" and self.settings.os != "Macos"
            options["OGRE_PLUGINS_POSTFIX"] = "_d" if add_postfix else ""

        cmake.configure(defs=options, build_dir='build')
        cmake.build(target='install')

    def imports(self):
        self.copy("*.dll", dst="build/bin", src="bin")
        self.copy("*.dylib", dst="build/bin", src="lib")
        self.copy("libluajit.*", dst="resources/luarocks/lib", src="lib")
        self.copy("luajit", dst="resources/luarocks/bin", src="bin")
        self.copy("*.*", dst="resources/luarocks/etc", src="etc")
        self.copy("*.lua", dst="resources/luarocks", src="share")
        self.copy("*.*", dst="resources/luarocks/tools", src="tools")
        self.copy("lua*.h*", dst="resources/luarocks/include", src="include")
        self.copy("lauxlib.h*", dst="resources/luarocks/include", src="include")

    def package(self):
        pass

    def package_info(self):
        self.cpp_info.libs = ["GsageCore"]
