OGRE_VERSION ?= 1.9.0
INPUT ?= OIS
WITH_LIBROCKET ?= True
UNAME_S := $(shell uname -s)
UNIT_CMD :=  cd ./build/bin/ && ./unit-tests
FUNCTIONAL_CMD := cd ./build/bin/ && ./functional-tests
RUN_CMD := cd ./build/bin/ && ./GsageExe
EDITOR_CMD := cd ./build/bin/ && ./GsageEditor
LOGS := ./build/bin/test.log
HEADERS := $(shell find ./ -name "*.h" -type f)
CPP := $(shell find ./ -name "*.cpp" -type f)
CMAKE := $(shell find ./ -name "*.txt" -type f)
TEST_PARAMS ?=
FILE_EXTENSION :=
POSTFIX :=

#using integration api_key/user by default
API_KEY ?= 3d14168da7de2092522ed90f72e9b6bf20db89e5
CONAN_USER ?= gsage-ci

ifeq ($(OS),Windows_NT)
FILE_EXTENSION := .exe
endif

ifeq ($(UNAME_S),Darwin)
UNIT_CMD := ./build/bin/unit-tests.app/Contents/MacOS/unit-tests
FUNCTIONAL_CMD := ./build/bin/functional-tests.app/Contents/MacOS/functional-tests
RUN_CMD := ./build/bin/GsageExe.app/Contents/MacOS/GsageExe
EDITOR_CMD := ./build/bin/GsageEditor.app/Contents/MacOS/GsageEditor
LOGS := ./build/bin/functional-tests.app/Contents/test.log
else
ifeq ($(CMAKE_BUILD_TYPE),Debug)
POSTFIX := _d
endif
endif

UNIT_CMD := $(UNIT_CMD)$(POSTFIX)$(FILE_EXTENSION)
FUNCTIONAL_CMD := $(FUNCTIONAL_CMD)$(POSTFIX)$(FILE_EXTENSION)
RUN_CMD := $(RUN_CMD)$(POSTFIX)$(FILE_EXTENSION)
EDITOR_CMD := $(EDITOR_CMD)$(POSTFIX)$(FILE_EXTENSION)

.repo: conanfile.py
	@if ! conan remote list | grep gsage; then conan remote add gsage https://api.bintray.com/conan/gsage/main --insert; conan user -p $(API_KEY) -r gsage $(CONAN_USER);	fi
	@touch .repo

.deps: .repo conanfile.py
	@conan install -g cmake -o gsage:with_ogre=$(OGRE_VERSION) -o gsage:with_input=$(INPUT) -o gsage:with_librocket=$(WITH_LIBROCKET) --build=outdated
	@touch .deps

upload-deps: .deps
	@conan upload "*" --all -r gsage -c

build: $(HEADERS) $(CPP) .deps resources/*.in $(CMAKE)
	@conan build .

unit: build
	@$(UNIT_CMD)

functional: build
	@$(FUNCTIONAL_CMD) -o gtest $(TEST_PARAMS) --exclude-tags benchmark --no-auto-insulate

benchmark: build
	@$(FUNCTIONAL_CMD) -o gtest $(TEST_PARAMS) --t benchmark

run: build
	@$(RUN_CMD)

editor: build
	@$(EDITOR_CMD)

ci: upload-deps build unit functional

all: build unit

clean:
	@rm -rf build

.PHONY: unit functional ci
