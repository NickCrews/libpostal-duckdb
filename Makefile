.PHONY: all clean format debug release duckdb_debug duckdb_release pull update

all: release

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJ_DIR := $(dir $(MKFILE_PATH))

ifeq ($(OS),Windows_NT)
	TEST_PATH="/test/Release/unittest.exe"
else
	TEST_PATH="/test/unittest"
endif

#### OSX config
OSX_BUILD_FLAG=
ifneq (${OSX_BUILD_ARCH}, "")
	OSX_BUILD_FLAG=-DOSX_BUILD_ARCH=${OSX_BUILD_ARCH}
endif

#### Enable Ninja as generator
ifeq ($(GEN),ninja)
	GENERATOR=-G "Ninja" -DFORCE_COLORED_OUTPUT=1
endif

#### Configuration for this extension
EXTENSION_NAME=POSTAL
EXTENSION_FLAGS=\
-DDUCKDB_EXTENSION_NAMES="postal" \
-DDUCKDB_EXTENSION_${EXTENSION_NAME}_PATH="$(PROJ_DIR)" \
-DDUCKDB_EXTENSION_${EXTENSION_NAME}_LOAD_TESTS=1 \
-DDUCKDB_EXTENSION_${EXTENSION_NAME}_INCLUDE_PATH="$(PROJ_DIR)src/include" \
-DDUCKDB_EXTENSION_${EXTENSION_NAME}_TEST_PATH="$(PROJ_DIR)test/sql"

#### Add more of the DuckDB in-tree extensions here that you need (also feel free to remove them when not needed)
EXTRA_EXTENSIONS_FLAG=-DBUILD_EXTENSIONS="tpch;visualizer"

BUILD_FLAGS=-DEXTENSION_STATIC_BUILD=1 $(EXTENSION_FLAGS) ${EXTRA_EXTENSIONS_FLAG} $(OSX_BUILD_FLAG) $(TOOLCHAIN_FLAGS)
CLIENT_FLAGS:=

LIBPOSTAL_CONFIGURE_FLAGS=--datadir=$(PROJ_DIR).libpostal_data
# I'm on apple M1, libpostal says need to use `--disable-sse2`
LIBPOSTAL_CONFIGURE_FLAGS+= --disable-sse2

N_CORES:=$(shell sysctl -n hw.ncpu)


#### Prepare libpostal submodule
# As of 2023-11-28, first need to install build deps as described at
# https://github.com/openvenues/libpostal
# For MacOS: brew install curl autoconf automake libtool pkg-config
libpostal-configure:
	cd libpostal && \
	./bootstrap.sh && \
	./configure $(LIBPOSTAL_CONFIGURE_FLAGS) && \
	make -j $(N_CORES) && \
	sudo make install

#### Main build
# For regular CLI build, we link the extension directly into the DuckDB executable
CLIENT_FLAGS=-DDUCKDB_EXTENSION_${EXTENSION_NAME}_SHOULD_LINK=1

debug:
	mkdir -p  build/debug && \
	cmake $(GENERATOR) $(BUILD_FLAGS) $(CLIENT_FLAGS) -DCMAKE_BUILD_TYPE=Debug -S ./duckdb/ -B build/debug && \
	cmake --build build/debug --config Debug --parallel $(N_CORES)

release:
	mkdir -p build/release && \
	cmake $(GENERATOR) $(BUILD_FLAGS)  $(CLIENT_FLAGS)  -DCMAKE_BUILD_TYPE=Release -S ./duckdb/ -B build/release && \
	cmake --build build/release --config Release --parallel $(N_CORES)

##### Client build
JS_BUILD_FLAGS=-DBUILD_NODE=1 -DDUCKDB_EXTENSION_${EXTENSION_NAME}_SHOULD_LINK=0
PY_BUILD_FLAGS=-DBUILD_PYTHON=1 -DDUCKDB_EXTENSION_${EXTENSION_NAME}_SHOULD_LINK=0

debug_js: CLIENT_FLAGS=$(JS_BUILD_FLAGS)
debug_js: debug
debug_python: CLIENT_FLAGS=$(PY_BUILD_FLAGS)
debug_python: debug
release_js: CLIENT_FLAGS=$(JS_BUILD_FLAGS)
release_js: release
release_python: CLIENT_FLAGS=$(PY_BUILD_FLAGS)
release_python: release

# Main tests
test: test_release
test_release: release
	./build/release/$(TEST_PATH) "$(PROJ_DIR)test/*"
test_debug: debug
	./build/debug/$(TEST_PATH) "$(PROJ_DIR)test/*"

#### Client tests
DEBUG_EXT_PATH='$(PROJ_DIR)build/debug/extension/postal/postal.duckdb_extension'
RELEASE_EXT_PATH='$(PROJ_DIR)build/release/extension/postal/postal.duckdb_extension'
test_js: test_debug_js
test_debug_js: debug_js
	cd duckdb/tools/nodejs && ${EXTENSION_NAME}_EXTENSION_BINARY_PATH=$(DEBUG_EXT_PATH) npm run test-path -- "../../../test/nodejs/**/*.js"
test_release_js: release_js
	cd duckdb/tools/nodejs && ${EXTENSION_NAME}_EXTENSION_BINARY_PATH=$(RELEASE_EXT_PATH) npm run test-path -- "../../../test/nodejs/**/*.js"
test_python: test_debug_python
test_debug_python: debug_python
	cd test/python && ${EXTENSION_NAME}_EXTENSION_BINARY_PATH=$(DEBUG_EXT_PATH) python3 -m pytest
test_release_python: release_python
	cd test/python && ${EXTENSION_NAME}_EXTENSION_BINARY_PATH=$(RELEASE_EXT_PATH) python3 -m pytest

#### Misc
format:
	find src/ -iname *.hpp -o -iname *.cpp | xargs clang-format --sort-includes=0 -style=file -i
	cmake-format -i CMakeLists.txt
update:
	git submodule update --remote --merge
pull:
	git submodule init
	git submodule update --recursive --remote

clean:
	rm -rf build
	rm -rf testext
	cd duckdb && make clean
	cd duckdb && make clean-python
