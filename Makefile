OGRE_VERSION ?= 2.1.0
LUA_VERSION ?= luajit-2.0.5
OIS_ENABLED ?= False
WITH_LIBROCKET ?= True
UNAME_S := $(shell uname -s)
IS_WINDOWS := 0
TEST_PARAMS ?=
FILE_EXTENSION :=
POSTFIX :=
PREFIX := ./

export CMAKE_BUILD_TYPE=Release

ifeq ($(BUILD_TYPE),debug)
	export CMAKE_BUILD_TYPE=Debug
endif

#using integration api_key/user by default
API_KEY ?= 3d14168da7de2092522ed90f72e9b6bf20db89e5
CONAN_USER ?= gsage-ci

ifeq ($(UNAME_S),MSYS_NT-10.0)
IS_WINDOWS := 1
endif

ifeq ($(OS),Windows_NT)
IS_WINDOWS := 1
endif

LOGS := ./build/bin/test.log
CMAKE_FUNCTIONS :=
ifeq ($(IS_WINDOWS),1)
PREFIX :=
SHELL = cmd
HEADERS := $(shell where /r . *.h)
CPP := $(shell where /r . *.cpp)
CPP := $(shell where /r . *.cpp)
CMAKE := $(shell where /r . CMakeLists.txt)
CONFIGS := $(shell where /r ./resources *.in)
CMAKE_FUNCTIONS := $(shell where /r . *.cmake)
ADD_REPO_CMD = conan remote list | findstr /I "gsage" && if %errorlevel% == 1 (conan remote add gsage https://api.bintray.com/conan/gsage/main --insert && conan user -p $(API_KEY) -r gsage $(CONAN_USER))
ADD_BINCRAFTERS_CMD = conan remote list | findstr /I "bincrafters" && if %errorlevel% == 1 (conan remote add bincrafters "https://api.bintray.com/conan/bincrafters/public-conan" --insert 1)
else
HEADERS := $(shell find ./ -name "*.h" -o -name "*.hpp" -type f)
CPP := $(shell find ./ -name "*.cpp" -o -name "*.cc" -type f)
CMAKE := $(shell find ./ -name "*.txt" -o -name "*.cmake" -type f)
CONFIGS := resources/*.in
ADD_REPO_CMD := if ! conan remote list | grep gsage; then conan remote add gsage https://api.bintray.com/conan/gsage/main --insert; conan user -p $(API_KEY) -r gsage $(CONAN_USER);  fi
ADD_BINCRAFTERS_CMD := if ! conan remote list | grep bincrafters; then conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan --insert 1;  fi
endif

UNIT_CMD :=  cd ./build/bin/ && $(PREFIX)unit-tests
FUNCTIONAL_CMD := cd ./build/bin/ && $(PREFIX)functional-tests
EDITOR_CMD := cd ./build/bin/ && $(PREFIX)gsage

ifeq ($(UNAME_S),Darwin)
UNIT_CMD := ./build/bin/unit-tests.app/Contents/MacOS/unit-tests
FUNCTIONAL_CMD := ./build/bin/functional-tests.app/Contents/MacOS/functional-tests
EDITOR_CMD := ./build/bin/gsage.app/Contents/MacOS/gsage
LOGS := ./build/bin/functional-tests.app/Contents/test.log
else
ifeq ($(CMAKE_BUILD_TYPE),Debug)
POSTFIX := _d
endif
endif

UNIT_CMD := $(UNIT_CMD)$(POSTFIX)$(FILE_EXTENSION)
FUNCTIONAL_CMD := $(FUNCTIONAL_CMD)$(POSTFIX)$(FILE_EXTENSION)
EDITOR_CMD := $(EDITOR_CMD)$(POSTFIX)$(FILE_EXTENSION)

.repo: conanfile.py
	$(ADD_REPO_CMD)
	$(ADD_BINCRAFTERS_CMD)
	@touch .repo

.deps: .repo conanfile.py
	conan install -g cmake -s build_type=$(CMAKE_BUILD_TYPE) -o gsage:with_ogre=$(OGRE_VERSION) -o gsage:with_ois=$(OIS_ENABLED) -o gsage:with_librocket=$(WITH_LIBROCKET) -o with_lua_version=$(LUA_VERSION)  --build=outdated .
	@touch .deps

upload-deps: .deps
	@conan upload "*" --all -r gsage -c

build: $(HEADERS) $(CPP) $(CMAKE) .deps $(CONFIGS) $(CMAKE_FUNCTIONS)
	@conan build .

unit: build
	@$(UNIT_CMD)

functional: build
	@$(FUNCTIONAL_CMD) -o gtest $(TEST_PARAMS) --exclude-tags benchmark --no-auto-insulate

benchmark: build
	@$(FUNCTIONAL_CMD) -o gtest $(TEST_PARAMS) --t benchmark --no-auto-insulate

editor: build
	@$(EDITOR_CMD)

ci: build unit functional

all: build unit

clean:
	@rm -rf build
	@rm .deps

.PHONY: unit functional ci
