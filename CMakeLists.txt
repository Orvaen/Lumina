cmake_minimum_required(VERSION 3.4...3.20 FATAL_ERROR)

project(Lumina VERSION 3.4.0 LANGUAGES C)

# CMake derives these from the project name, i.e. Lumina_SOURCE_DIR and friends.
# The rest of the build uses the short LUM_ prefix, matching the C API, so the
# generated variables are aliased here once.
set(LUM_SOURCE_DIR   "${Lumina_SOURCE_DIR}")
set(LUM_BINARY_DIR   "${Lumina_BINARY_DIR}")
set(LUM_VERSION      "${Lumina_VERSION}")
set(LUM_VERSION_MAJOR "${Lumina_VERSION_MAJOR}")
set(LUM_VERSION_MINOR "${Lumina_VERSION_MINOR}")
set(LUM_VERSION_PATCH "${Lumina_VERSION_PATCH}")

set(CMAKE_LEGACY_CYGWIN_WIN32 OFF)

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

if (POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(LUM_STANDALONE TRUE)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(LUM_BUILD_EXAMPLES "Build the Lumina example programs" ${LUM_STANDALONE})
option(LUM_BUILD_TESTS "Build the Lumina test programs" ${LUM_STANDALONE})
option(LUM_BUILD_DOCS "Build the Lumina documentation" ON)
option(LUM_INSTALL "Generate installation target" ON)

include(GNUInstallDirs)
include(CMakeDependentOption)

if (LUM_USE_OSMESA)
    message(FATAL_ERROR "LUM_USE_OSMESA has been removed; set the LUM_PLATFORM init hint")
endif()

cmake_dependent_option(LUM_BUILD_WIN32 "Build support for Win32" ON "WIN32" OFF)
cmake_dependent_option(LUM_BUILD_COCOA "Build support for Cocoa" ON "APPLE" OFF)
cmake_dependent_option(LUM_BUILD_X11 "Build support for X11" ON "UNIX;NOT APPLE" OFF)
cmake_dependent_option(LUM_BUILD_WAYLAND "Build support for Wayland"
    "${LUM_USE_WAYLAND}" "UNIX;NOT APPLE" OFF)

cmake_dependent_option(LUM_USE_HYBRID_HPG "Force use of high-performance GPU on hybrid systems" OFF
                       "WIN32" OFF)
cmake_dependent_option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON
                       "MSVC" OFF)

set(LUM_LIBRARY_TYPE "${LUM_LIBRARY_TYPE}" CACHE STRING
    "Library type override for Lumina (SHARED, STATIC, OBJECT, or empty to follow BUILD_SHARED_LIBS)")

if (LUM_LIBRARY_TYPE)
    if (LUM_LIBRARY_TYPE STREQUAL "SHARED")
        set(LUM_BUILD_SHARED_LIBRARY TRUE)
    else()
        set(LUM_BUILD_SHARED_LIBRARY FALSE)
    endif()
else()
    set(LUM_BUILD_SHARED_LIBRARY ${BUILD_SHARED_LIBS})
endif()

list(APPEND CMAKE_MODULE_PATH "${LUM_SOURCE_DIR}/CMake/modules")

find_package(Threads REQUIRED)

if (LUM_BUILD_DOCS)
    set(DOXYGEN_SKIP_DOT TRUE)
    find_package(Doxygen)
endif()

#--------------------------------------------------------------------
# Report backend selection
#--------------------------------------------------------------------
if (LUM_BUILD_WIN32)
    message(STATUS "Including Win32 support")
endif()
if (LUM_BUILD_COCOA)
    message(STATUS "Including Cocoa support")
endif()
if (LUM_BUILD_WAYLAND)
    message(STATUS "Including Wayland support")
endif()
if (LUM_BUILD_X11)
    message(STATUS "Including X11 support")
endif()

#--------------------------------------------------------------------
# Apply Microsoft C runtime library option
# This is here because it also applies to tests and examples
#--------------------------------------------------------------------
if (MSVC AND NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
    if (CMAKE_VERSION VERSION_LESS 3.15)
        foreach (flag CMAKE_C_FLAGS
                      CMAKE_C_FLAGS_DEBUG
                      CMAKE_C_FLAGS_RELEASE
                      CMAKE_C_FLAGS_MINSIZEREL
                      CMAKE_C_FLAGS_RELWITHDEBINFO)

            if (flag MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if (flag MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()

        endforeach()
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------
include(CMakePackageConfigHelpers)

set(LUM_CONFIG_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/lumina")

configure_package_config_file(CMake/luminaConfig.cmake.in
                              src/luminaConfig.cmake
                              INSTALL_DESTINATION "${LUM_CONFIG_PATH}"
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(src/luminaConfigVersion.cmake
                                 VERSION ${LUM_VERSION}
                                 COMPATIBILITY SameMajorVersion)

#--------------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------------
add_subdirectory(src)

if (LUM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (LUM_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (DOXYGEN_FOUND AND LUM_BUILD_DOCS)
    add_subdirectory(docs)
endif()

#--------------------------------------------------------------------
# Install files other than the library
# The library is installed by src/CMakeLists.txt
#--------------------------------------------------------------------
if (LUM_INSTALL)
    install(DIRECTORY include/Lumina DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN lumina.h PATTERN lumina_native.h)

    install(FILES "${LUM_BINARY_DIR}/src/luminaConfig.cmake"
                  "${LUM_BINARY_DIR}/src/luminaConfigVersion.cmake"
            DESTINATION "${LUM_CONFIG_PATH}")

    install(EXPORT luminaTargets FILE luminaTargets.cmake
            EXPORT_LINK_INTERFACE_LIBRARIES
            DESTINATION "${LUM_CONFIG_PATH}")
    install(FILES "${LUM_BINARY_DIR}/src/lumina.pc"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

    if (DOXYGEN_FOUND AND LUM_BUILD_DOCS)
        install(DIRECTORY "${LUM_BINARY_DIR}/docs/html"
                DESTINATION "${CMAKE_INSTALL_DOCDIR}")
    endif()

    # Only generate this target if no higher-level project already has
    if (NOT TARGET uninstall)
        configure_file(CMake/cmake_uninstall.cmake.in
                       cmake_uninstall.cmake IMMEDIATE @ONLY)

        add_custom_target(uninstall
                          "${CMAKE_COMMAND}" -P
                          "${LUM_BINARY_DIR}/cmake_uninstall.cmake")
        set_target_properties(uninstall PROPERTIES FOLDER "Lumina")
    endif()
endif()

