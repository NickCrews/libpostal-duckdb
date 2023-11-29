cmake_minimum_required(VERSION 2.8.12)

# Set extension name here
set(TARGET_NAME postal)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})
include_directories(src/include)

find_package(PkgConfig REQUIRED)
pkg_search_module(LIBPOSTAL REQUIRED IMPORTED_TARGET libpostal)
include_directories(PkgConfig::LIBPOSTAL)

set(EXTENSION_SOURCES src/postal_extension.cpp)

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

target_link_libraries(${EXTENSION_NAME} PkgConfig::LIBPOSTAL)
target_link_libraries(${LOADABLE_EXTENSION_NAME} PkgConfig::LIBPOSTAL)

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
