########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.12)
project(xdpiface)
enable_language(C)
enable_testing()

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# Select flags
if(MSVC)
    SET(CMAKE_C_FLAGS_RELEASE "/O2")
else()
    SET(CMAKE_C_FLAGS_RELEASE "-O3")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/builds/cmake/Modules)

# Will be used to add flags to pkg-config useful when apps want to statically link
set(pkg_config_libs_private "")
set(pkg_config_names_private "")

########################################################################
# options
########################################################################
OPTION (ENABLE_DRAFTS "Build and install draft classes and methods" ON)
IF (ENABLE_DRAFTS)
    ADD_DEFINITIONS (-DXDPIFACE_BUILD_DRAFT_API)
ENDIF (ENABLE_DRAFTS)

########################################################################
# platform.h
########################################################################
include(CheckIncludeFile)
CHECK_INCLUDE_FILE("linux/wireless.h" HAVE_LINUX_WIRELESS_H)
CHECK_INCLUDE_FILE("net/if_media.h" HAVE_NET_IF_MEDIA_H)

include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS("getifaddrs" HAVE_GETIFADDRS)
CHECK_FUNCTION_EXISTS("freeifaddrs" HAVE_FREEIFADDRS)

include(CheckIncludeFiles)
check_include_files("sys/socket.h;net/if.h" HAVE_NET_IF_H)
if (NOT HAVE_NET_IF_H)
    CHECK_INCLUDE_FILE("net/if.h" HAVE_NET_IF_H)
endif()

file(REMOVE "${SOURCE_DIR}/src/platform.h")

file(WRITE "${PROJECT_BINARY_DIR}/platform.h.in" "
#cmakedefine HAVE_LINUX_WIRELESS_H
#cmakedefine HAVE_NET_IF_H
#cmakedefine HAVE_NET_IF_MEDIA_H
#cmakedefine HAVE_GETIFADDRS
#cmakedefine HAVE_FREEIFADDRS
")

configure_file("${PROJECT_BINARY_DIR}/platform.h.in" "${PROJECT_BINARY_DIR}/platform.h")

set(MORE_LIBRARIES xdp bpf)

list(APPEND CMAKE_MODULE_PATH "${SOURCE_DIR}")
set(OPTIONAL_LIBRARIES)
set(OPTIONAL_LIBRARIES_STATIC)

########################################################################
# version
########################################################################
set(XDPIFACE_VERSION_MAJOR 1)
set(XDPIFACE_VERSION_MINOR 1)
set(XDPIFACE_VERSION_PATCH 0)
set(XDPIFACE_VERSION "${XDPIFACE_VERSION_MAJOR}.${XDPIFACE_VERSION_MINOR}.${XDPIFACE_VERSION_PATCH}")
message(STATUS "Detected XDPIFACE Version - ${XDPIFACE_VERSION}")


########################################################################
# includes
########################################################################
set (xdpiface_headers
    include/xdpiface_library.h
    include/xdpiface.h
)

IF (ENABLE_DRAFTS)
    list(APPEND xdpiface_headers
        include/xdp_iface.h
        include/xdp_sock.h
    )
ENDIF (ENABLE_DRAFTS)

source_group ("Header Files" FILES ${xdpiface_headers})
install(FILES ${xdpiface_headers} DESTINATION include)

########################################################################
# library
########################################################################


include_directories("${SOURCE_DIR}/src" "${SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}")
set (xdpiface_sources
)

IF (ENABLE_DRAFTS)
    list (APPEND xdpiface_sources
        src/xdp_iface.c
        src/xdp_sock.c
        src/xdp_log.c
    )
ENDIF (ENABLE_DRAFTS)

IF (ENABLE_DRAFTS)
    list (APPEND xdpiface_sources
        src/xdpiface_private_selftest.c
    )
ENDIF (ENABLE_DRAFTS)

source_group("Source Files" FILES ${xdpiface_sources})


option(XDPIFACE_BUILD_SHARED "Whether or not to build the shared object" ON)
option(XDPIFACE_BUILD_STATIC "Whether or not to build the static archive" ON)

if (NOT XDPIFACE_BUILD_SHARED AND NOT XDPIFACE_BUILD_STATIC)
  message(FATAL_ERROR "Neither static nor shared library build enabled")
endif()

IF (NOT MSVC)
  # avoid building everything twice for shared + static
  # only on *nix, as Windows needs different preprocessor defines in static builds
  add_library (xdpiface_objects OBJECT ${xdpiface_sources})
  set_property(TARGET xdpiface_objects PROPERTY POSITION_INDEPENDENT_CODE ON)
ENDIF (NOT MSVC)

# shared
if (XDPIFACE_BUILD_SHARED)
  IF (APPLE)
    add_library(xdpiface SHARED ${xdpiface_sources})
  ELSE (APPLE)
    IF (MSVC)
      add_library(xdpiface SHARED ${xdpiface_sources})
    ELSE (MSVC)
      add_library(xdpiface SHARED $<TARGET_OBJECTS:xdpiface_objects>)
    ENDIF (MSVC)
  ENDIF(APPLE)

  set_target_properties (xdpiface PROPERTIES
    PUBLIC_HEADER "${public_headers}"
    DEFINE_SYMBOL "XDPIFACE_EXPORTS"
    SOVERSION "0"
    VERSION "${XDPIFACE_VERSION}"
    COMPILE_DEFINITIONS "DLL_EXPORT"
    OUTPUT_NAME "xdpiface"
    PREFIX "lib"
  )

  target_link_libraries(xdpiface
    PUBLIC ${MORE_LIBRARIES}
  )

  install(TARGETS xdpiface
    EXPORT xdpiface-targets
    LIBRARY DESTINATION "lib${LIB_SUFFIX}" # .so file
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}" # .lib file
    RUNTIME DESTINATION bin                # .dll file
  )

  target_include_directories(xdpiface
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
endif()

# static
if (XDPIFACE_BUILD_STATIC)
  IF (APPLE)
    add_library(xdpiface-static STATIC ${xdpiface_sources})
  ELSE (APPLE)
    IF (MSVC)
      add_library(xdpiface-static STATIC ${xdpiface_sources})
    ELSE (MSVC)
      add_library(xdpiface-static STATIC $<TARGET_OBJECTS:xdpiface_objects>)
    ENDIF (MSVC)
  ENDIF (APPLE)

  set_target_properties(xdpiface-static PROPERTIES
    PUBLIC_HEADER "${public_headers}"
    COMPILE_DEFINITIONS "XDPIFACE_STATIC"
    OUTPUT_NAME "xdpiface"
    PREFIX "lib"
  )

  target_link_libraries(xdpiface-static
    PUBLIC ${MORE_LIBRARIES}
  )

  install(TARGETS xdpiface-static
    EXPORT xdpiface-targets
    LIBRARY DESTINATION "lib${LIB_SUFFIX}" # .so file
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}" # .lib file
    RUNTIME DESTINATION bin                # .dll file
  )

  target_include_directories(xdpiface-static
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
  target_compile_definitions(xdpiface-static
    PUBLIC XDPIFACE_STATIC
  )

endif()



########################################################################
# pkgconfig
########################################################################
set (VERSION "1.1.0")
set (prefix "${CMAKE_INSTALL_PREFIX}")
set (exec_prefix "\${prefix}")
set (libdir "\${prefix}/lib${LIB_SUFFIX}")
set (includedir "\${prefix}/include")
IF (ENABLE_DRAFTS)
    set (pkg_config_defines "-DXDPIFACE_BUILD_DRAFT_API=1")
ELSE (ENABLE_DRAFTS)
    set (pkg_config_defines "")
ENDIF (ENABLE_DRAFTS)
configure_file(
    "${SOURCE_DIR}/src/libxdpiface.pc.in"
    "${SOURCE_DIR}/src/libxdpiface.pc"
@ONLY)

install(
    FILES "${SOURCE_DIR}/src/libxdpiface.pc"
    DESTINATION "lib${LIB_SUFFIX}/pkgconfig"
)

########################################################################
# installer
########################################################################
include(CMakePackageConfigHelpers)
if (WIN32)
  set(CMAKECONFIG_INSTALL_DIR "CMake" CACHE STRING "install path for xdpifaceConfig.cmake")
else()
  # GNUInstallDirs "DATADIR" wrong here; CMake search path wants "share".
  set(CMAKECONFIG_INSTALL_DIR "share/cmake/xdpiface" CACHE STRING "install path for xdpifaceConfig.cmake")
endif()

if (NOT CMAKE_VERSION VERSION_LESS 3.0)
  export(EXPORT xdpiface-targets
         FILE "${CMAKE_CURRENT_BINARY_DIR}/xdpifaceTargets.cmake")
endif()
configure_package_config_file(builds/cmake/Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/xdpifaceConfig.cmake"
                              INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/xdpifaceConfigVersion.cmake
                                 VERSION ${XDPIFACE_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(EXPORT xdpiface-targets
        FILE xdpifaceTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xdpifaceConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/xdpifaceConfigVersion.cmake
              DESTINATION ${CMAKECONFIG_INSTALL_DIR})

########################################################################
# executables
########################################################################
add_executable(
    xdpiface_selftest
    "${SOURCE_DIR}/src/xdpiface_selftest.c"
)
if (TARGET xdpiface)
target_link_libraries(
    xdpiface_selftest
    xdpiface
    ${OPTIONAL_LIBRARIES}
)
endif()
if (NOT TARGET xdpiface AND TARGET xdpiface-static)
target_link_libraries(
    xdpiface_selftest
    xdpiface-static
    ${OPTIONAL_LIBRARIES}
    ${OPTIONAL_LIBRARIES_STATIC}
)
endif()

add_executable(
    xdpiface_app
    "${SOURCE_DIR}/src/xdp_iface_app.c"
)
target_link_libraries(xdpiface_app xdpiface)

########################################################################
# tests
########################################################################
set(CLASSTEST_TIMEOUT 60 CACHE STRING "Timeout of the selftest of a class")
set(TOTAL_TIMEOUT 600 CACHE STRING "Timout of the total testsuite")

if(UNIX)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-reachable=yes
                                     --error-exitcode=1
                                     --suppressions=src/.valgrind.supp")
endif()

set(TEST_CLASSES
)

IF (ENABLE_DRAFTS)
    list (APPEND TEST_CLASSES
    xdp_iface
    xdp_sock
    )
ENDIF (ENABLE_DRAFTS)


if (NOT TARGET copy-selftest-ro)
    add_custom_target(
        copy-selftest-ro ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/src/selftest-ro ${PROJECT_BINARY_DIR}/src/selftest-ro
    )
endif()

if (NOT TARGET make-selftest-rw)
    add_custom_target(
        make-selftest-rw ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/src/selftest-rw
    )
endif()


set_directory_properties(
    PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_BINARY_DIR}/src/selftest-ro;${PROJECT_BINARY_DIR}/src/selftest-rw"
)

foreach(TEST_CLASS ${TEST_CLASSES})
    add_test(
        NAME ${TEST_CLASS}
        COMMAND xdpiface_selftest --continue --verbose --test ${TEST_CLASS}
    )
    IF (WIN32 AND CMAKE_PREFIX_PATH)
        file(TO_NATIVE_PATH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH_WIN)
        set_tests_properties(
            ${TEST_CLASS}
            PROPERTIES ENVIRONMENT "PATH=%PATH%\;${CMAKE_PREFIX_PATH_WIN}\\bin"
        )
    ENDIF (WIN32 AND CMAKE_PREFIX_PATH)
    set_tests_properties(
        ${TEST_CLASS}
        PROPERTIES TIMEOUT ${CLASSTEST_TIMEOUT}
    )
    set_tests_properties(
        ${TEST_CLASS}
        PROPERTIES DEPENDS "copy-selftest-ro;make-selftest-rw"
    )
endforeach(TEST_CLASS)

include(CTest)

########################################################################
# XDP BPF SOCK
########################################################################
add_subdirectory(xdp_sock_bpf)

########################################################################
# Bindings
########################################################################
add_subdirectory(bindings)

########################################################################
# cleanup
########################################################################
if (NOT TARGET distclean)
    add_custom_target (distclean @echo Cleaning for source distribution)
endif()

set(cmake_generated ${PROJECT_BINARY_DIR}/CMakeCache.txt
                    ${PROJECT_BINARY_DIR}/cmake_install.cmake
                    ${PROJECT_BINARY_DIR}/Makefile
                    ${PROJECT_BINARY_DIR}/CMakeFiles
                    ${PROJECT_BINARY_DIR}/CTestTestfile.cmake
                    ${PROJECT_BINARY_DIR}/DartConfiguration.tcl
                    ${PROJECT_BINARY_DIR}/Testing
                    ${PROJECT_BINARY_DIR}/compile_commands.json
                    ${PROJECT_BINARY_DIR}/platform.h
                    ${PROJECT_BINARY_DIR}/src/libxdpiface.pc
                    ${PROJECT_BINARY_DIR}/src/libxdpiface.so
                    ${PROJECT_BINARY_DIR}/src/xdpiface_selftest
                    ${PROJECT_BINARY_DIR}/src/xdpiface_selftest
)

add_custom_command(
    DEPENDS clean
    COMMENT "distribution clean"
    COMMAND rm
    ARGS    -rf CMakeTmp ${cmake_generated}
    TARGET  distclean
)

include(ClangFormat OPTIONAL)

########################################################################
# summary
########################################################################
message ("")
message (STATUS "******************* Configuration Summary *******************")
message (STATUS "General:")
message (STATUS "  Version           :   ${VERSION}")
message (STATUS "  System            :   ${CMAKE_SYSTEM_NAME}")
message (STATUS "  C compiler        :   ${CMAKE_C_COMPILER}")
message (STATUS "  Debug C flags     :   ${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS}")
message (STATUS "  Release C flags   :   ${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS}")
message (STATUS "  Build type        :   ${CMAKE_BUILD_TYPE}")
message (STATUS "  Static build      :   ${XDPIFACE_BUILD_STATIC}")
message (STATUS "  Shared build      :   ${XDPIFACE_BUILD_SHARED}")
IF (ENABLE_DRAFTS)
message (STATUS "  Draft API         :   Yes")
ELSE (ENABLE_DRAFTS)
message (STATUS "  Draft API         :   No")
ENDIF (ENABLE_DRAFTS)
message (STATUS "")
message (STATUS "Dependencies:")
include(FeatureSummary)
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
message (STATUS "")
message (STATUS "Install:")
message (STATUS "  Install prefix    :" "${CMAKE_INSTALL_PREFIX}")
message (STATUS "")
message (STATUS "************************* Options ***************************")
message (STATUS "Options:")
message (STATUS "  Use the Draft API (default = yes):")
message (STATUS "  -DENABLE-DRAFTS=[yes|no]")
message (STATUS "")
message (STATUS "*************************************************************")
message (STATUS "Configuration complete! Now procced with:")
message (STATUS "  'make'                 compile the project")
message (STATUS "  'make test'            run the project's selftest")
message (STATUS "  'make install'         install the project to ${CMAKE_INSTALL_PREFIX}")
message (STATUS "")
message (STATUS "Further options are:")
message (STATUS "  'ctest -V              run test with verbose logging")
message (STATUS "  'ctest -R <test_name>' run a specific test")
message (STATUS "  'ctest -T memcheck'    run the project's selftest with")
message (STATUS "                         valgrind to check for memory leaks")
message (STATUS "")

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
