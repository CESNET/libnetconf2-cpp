#
# Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
#
# Written by Václav Kubernát <kubernat@cesnet.cz>
#

cmake_minimum_required(VERSION 3.19)
project(libnetconf2-cpp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS_DEBUG "-Werror ${CMAKE_CXX_FLAGS_DEBUG}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Woverloaded-virtual -Wimplicit-fallthrough -Wsuggest-override ${CMAKE_CXX_FLAGS}")

add_custom_target(libnetconf2-cpp-version-cmake
        cmake/ProjectGitVersion.cmake
        cmake/ProjectGitVersionRunner.cmake
        )
include(cmake/ProjectGitVersion.cmake)
prepare_git_version(LIBYANG_CPP_VERSION "0.0")

find_package(Doxygen)
option(WITH_DOCS "Create and install internal documentation (needs Doxygen)" ${DOXYGEN_FOUND})
option(BUILD_SHARED_LIBS "By default, shared libs are enabled. Turn off for a static build." ON)

find_package(PkgConfig)
pkg_check_modules(LIBYANG_CPP REQUIRED libyang-cpp>=2 IMPORTED_TARGET)
pkg_check_modules(LIBNETCONF2 REQUIRED libnetconf2>=3.0.17 IMPORTED_TARGET)
set(LIBNETCONF2_CPP_PKG_VERSION "2")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)

add_library(netconf2-cpp
    src/netconf-client.cpp
    )

target_link_libraries(netconf2-cpp PUBLIC PkgConfig::LIBYANG_CPP PRIVATE PkgConfig::LIBNETCONF2)

if(BUILD_TESTING)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads)
    find_package(doctest 2.4.8 REQUIRED)
    find_package(Boost REQUIRED)

    add_library(DoctestIntegration STATIC
        tests/doctest-integration.cpp
        )
    target_include_directories(DoctestIntegration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests/ ${CMAKE_CURRENT_SOURCE_DIR}/src/ "${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(DoctestIntegration doctest::doctest)
    target_compile_definitions(DoctestIntegration PUBLIC DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vars.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/test_vars.hpp" @ONLY)

    add_library(mock_server STATIC
        tests/mock_server.cpp
        )
    target_link_libraries(mock_server DoctestIntegration)

    function(libnetconf2_cpp_test name)
        add_executable(test_${name}
            tests/${name}.cpp
            )
        target_link_libraries(test_${name} DoctestIntegration netconf2-cpp Threads::Threads mock_server)

        add_test(test_${name} test_${name})
    endfunction()

    libnetconf2_cpp_test(client)
endif()

if(WITH_DOCS)
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)
    add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
            SOURCES ${doxyfile_in}
            )
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libnetconf2-cpp.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/libnetconf2-cpp.pc" @ONLY)

install(TARGETS netconf2-cpp)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/libnetconf2-cpp" TYPE INCLUDE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libnetconf2-cpp.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
