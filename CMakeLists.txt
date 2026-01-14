cmake_minimum_required(VERSION 3.14)
project(Argon VERSION 2.0.0 LANGUAGES CXX)

add_library(Argon INTERFACE)
add_library(Argon::Argon ALIAS Argon)

target_compile_features(Argon INTERFACE cxx_std_23)
target_sources(Argon
        INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
        include/argon/argon.hpp
)

include(GNUInstallDirs)

install(
        TARGETS Argon
        EXPORT ArgonTargets
        FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
        EXPORT ArgonTargets
        FILE ArgonTargets.cmake
        NAMESPACE Argon::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ArgonConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ArgonConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Argon
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ArgonConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ArgonConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ArgonConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}
)

enable_testing()
option(ARGON_BUILD_TESTS "Build Argon tests" ON)

if (ARGON_BUILD_TESTS)
    add_subdirectory(tests)
endif()
