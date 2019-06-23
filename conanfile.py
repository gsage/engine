import os
import re

from distutils.version import LooseVersion

from conans import ConanFile, CMake, tools


# read version
git = tools.Git(".")
git.run("pull origin master --tags")
version = git.run("describe --tags")


class GsageConan(ConanFile):
    name = "gsage"
    version = version
    license = "MIT"
    url = "https://github.com/gsage/engine"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "with_ogre": ["1.9.0", "2.1.0", "disabled"],
        "with_ois": [True, False],
        "with_librocket": [True, False],
        "with_lua_version": ["luajit-2.0.5", "luajit-2.1.0", "lua-5.1"],
        "with_recast": [True, False],
        "with_metal": [True, False]
    }
    default_options = (
        "shared=False",
        "with_ois=False",
        "with_ogre=disabled",
        "with_librocket=False",
        "with_lua_version=luajit-2.0.5",
        "with_recast=True",
        "with_metal=False",
        "cef:use_sandbox=False",
        "Poco:enable_json=False",
        "Poco:enable_mongodb=False",
        "Poco:enable_data_sqlite=False",
    )
    generators = "cmake"
    requires = (
        ("zlib/1.2.11@lasote/stable",),
        ("msgpack/2.1.3@gsage/master",),
        ("SDL2/2.0.8@gsage/master",),
        ("gtest/1.8.1@lasote/stable",),
        ("inja/2.0.1@gsage/master",),
        ("jsonformoderncpp/3.5.0@gsage/master",),
        ("Poco/1.9.0@pocoproject/stable",),
    )

    def source(self):
        self.run("git clone https://github.com/gsage/engine")

    def config_options(self):
        lua, version = str(self.options.with_lua_version).split("-")

        if self.settings.os == "Windows":
            # only MD is supported on Windows
            self.settings.compiler.runtime = "MD"

        lua_package = "{}-rocks".format(lua)

        self.requires.add("{}/{}@gsage/master".format(lua_package, version))
        self.options[lua_package].shared = False

        self.options["gtest"].shared = False
        if self.settings.os == "Macos":
            self.options["SDL2"].x11_video = False
            self.requires.add("cef/3.3239.1709.g093cae4@gsage/master")
        else:
            # 3440 has scaling issues on OSX
            self.requires.add("cef/3.3440.1806.g65046b7@gsage/master")

        if self.options.with_ogre != "disabled":
            self.requires.add("OGRE/{}@gsage/master".format(self.options.with_ogre))
            self.options["OGRE"].shared = self.settings.os == "Windows"

            if re.match("^1\\..*$", str(self.options.with_ogre)):
                self.options["OGRE"].with_boost = self.settings.os != "Windows"

            if LooseVersion(str(self.options.with_ogre)) >= LooseVersion("2.1.0") \
               and LooseVersion(str(self.settings.compiler.version)) > LooseVersion("9.0"):
                if self.settings.os == "Macos":
                    self.options["OGRE"].with_metal = True
                    self.options.with_metal = True

        if self.options.with_ois:
            self.requires.add("OIS/1.3@gsage/master")
        if self.options.with_librocket:
            self.requires.add("libRocket/1.3.0@gsage/master")
            self.options["libRocket"].with_lua_bindings = True
        if self.options.with_recast:
            self.requires.add("recast/latest@gsage/master")

    def build(self):
        cmake = CMake(self)
        (major_version, minor_version, patch_version) = version.split(".")
        parts = patch_version.split("-")
        patch_version = parts[0]

        options = {
            "CMAKE_INSTALL_PREFIX": "./sdk",
            "CMAKE_BUILD_TYPE": self.settings.build_type,
            "GSAGE_VERSION_MAJOR": major_version,
            "GSAGE_VERSION_MINOR": minor_version,
            "GSAGE_VERSION_PATCH": patch_version,
            "GSAGE_VERSION_BUILD": os.environ.get("APPVEYOR_BUILD_VERSION", os.environ.get("GSAGE_VERSION_BUILD", "-".join(parts[1:]))),
        }

        if self.options.with_ogre != "disabled":
            major, minor, patch = str(self.options.with_ogre).split(".")
            options["OGRE_VERSION_MAJOR"] = major
            options["OGRE_VERSION_MINOR"] = minor
            options["OGRE_VERSION_PATCH"] = patch

        if self.options.with_metal:
            options["WITH_METAL"] = True

        if self.settings.os == "Macos":
            options["CMAKE_OSX_ARCHITECTURES"] = "x86_64"

        if self.options.with_ois:
            options["WITH_OIS"] = self.options.with_ois

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
        self.copy("*.so", dst="build/bin", src="bin")
        self.copy("*.dylib", dst="build/bin", src="lib")
        self.copy("*.bin", dst="build/bin", src="bin")
        self.copy("*.dat", dst="build/bin", src="bin")
        self.copy("*.pak", dst="build/bin", src="bin")
        self.copy("locales", dst="build/bin", src="bin")
        self.copy("chrome-sandbox", dst="build/bin", src="bin")
        self.copy("*.framework/*", dst="build/bin/Frameworks", src="lib")
        self.copy("libluajit.*", dst="resources/luarocks/lib", src="lib")
        self.copy("luajit", dst="resources/luarocks/bin", src="bin")
        self.copy("*.*", dst="resources/luarocks/etc", src="etc")
        self.copy("*.lua", dst="resources/luarocks", src="share")
        self.copy("*.*", dst="resources/luarocks/tools", src="tools")
        self.copy("lua*.h*", dst="resources/luarocks/include", src="include")
        self.copy("lauxlib.h*", dst="resources/luarocks/include", src="include")

        self.copy("*.dll", dst="build/bundle", src="bin")
        self.copy("*.so", dst="build/bundle", src="bin")
        self.copy("*.so", dst="build/bundle", src="lib")
        self.copy("chrome-sandbox", dst="build/bundle", src="bin")
        self.copy("*.dylib", dst="build/bundle", src="lib")
        self.copy("*.bin", dst="build/bundle", src="bin")
        self.copy("*.dat", dst="build/bundle", src="bin")
        self.copy("*.pak", dst="build/bundle", src="bin")
        self.copy("locales", dst="build/bundle", src="bin")

    def package(self):
        pass

    def package_info(self):
        self.cpp_info.libs = ["GsageCore"]
