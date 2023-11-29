cmake_minimum_required(VERSION 2.8.12)

# Set extension name here
set(TARGET_NAME libpostal)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})
include_directories(src/include)
# use `pkg-config --libs libpostal` to find the correct linker flags
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPOSTAL REQUIRED libpostal)
include_directories(${LIBPOSTAL_INCLUDE_DIRS})

set(EXTENSION_SOURCES src/libpostal_extension.cpp)

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# target_link_libraries(${EXTENSION_NAME} ${LIBPOSTAL_LIBRARIES})
# target_link_libraries(${LOADABLE_EXTENSION_NAME} ${LIBPOSTAL_LIBRARIES})
target_link_libraries(${EXTENSION_NAME} postal)
target_link_libraries(${LOADABLE_EXTENSION_NAME} postal)

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
