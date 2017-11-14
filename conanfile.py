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
        "with_ogre": ["1.9.0", "disabled"],
        "with_input": ["nothing", "OIS"],
        "with_librocket": [True, False]
    }
    default_options = (
        "shared=False",
        "with_input=nothing",
        "with_ogre=disabled",
        "with_librocket=False"
    )
    generators = "cmake"
    requires = (
        ("luajit-rocks/2.0.5@gsage/master",),
        ("msgpack/2.1.3@gsage/master",),
        ("SDL2/2.0.5@gsage/master",),
    )

    def source(self):
        self.run("git clone https://github.com/gsage/engine")

    def config_options(self):
        if self.settings.os == "Macos":
            self.options["SDL2"].x11_video = False

        if self.options.with_ogre == "1.9.0":
            self.requires.add("OGRE/1.9.0@gsage/master", private=True)
            self.options["OGRE"].shared = False
        if self.options.with_input == "OIS":
            repo = "hilborn/stable"

            # hilborn/stable does not work for osx
            if self.settings.os == "Macos":
                repo = "gsage/master"

            self.requires.add("OIS/1.3@{}".format(repo), private=False)
        if self.options.with_librocket:
            self.requires.add("libRocket/1.3.0@gsage/master", private=False)
            self.options["libRocket"].with_lua_bindings = True

    def build(self):
        cmake = CMake(self)
        options = {
            "CMAKE_INSTALL_PREFIX": "./sdk",
            "CMAKE_BUILD_TYPE": os.environ.get("CMAKE_BUILD_TYPE", "Release")
        }
        if self.settings.os == "Macos":
            options["CMAKE_OSX_ARCHITECTURES"] = "x86_64"

        if self.options.with_ogre != "disabled":
            options["OGRE_FOUND"] = True
        cmake.configure(defs=options, build_dir='build')
        cmake.build(target='install')

    def imports(self):
        self.copy("*.dll", dst="build/bin", src="bin")
        self.copy("*.dylib", dst="build/bin", src="lib")
        self.copy("libluajit.*", dst="resources/luarocks/lib", src="lib")
        self.copy("luajit", dst="resources/luarocks/bin", src="bin")
        self.copy("*.*", dst="resources/luarocks/etc", src="etc")
        self.copy("*.lua", dst="resources/luarocks", src="share")
        self.copy("lua*.h*", dst="resources/luarocks/include", src="include")
        self.copy("lauxlib.h*", dst="resources/luarocks/include", src="include")

    def package(self):
        pass

    def package_info(self):
        self.cpp_info.libs = ["GsageCore"]
