cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project("morton-nd"
        VERSION 4.0.0
        DESCRIPTION
        "A header-only constexpr library for fast Morton encoding/decoding in N dimensions."
        HOMEPAGE_URL "morton.codes"
        LANGUAGES CXX)

# Global variable for the MortonND library name.
set(MORTON_ND_LIBRARY_NAME "MortonND" CACHE INTERNAL "")

# Include CTest only if this is the root cmake proj. Otherwise,
# we're being consumed as a library.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# Sets CMAKE_INSTALL_* vars to canonical GNU directories.
include(GNUInstallDirs)

add_library(${MORTON_ND_LIBRARY_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${MORTON_ND_LIBRARY_NAME} ALIAS ${MORTON_ND_LIBRARY_NAME})

# Adding the install interface generator expression makes sure that the include
# files are picked up from the GNUInstallDirs location when installed.
target_include_directories(
        ${MORTON_ND_LIBRARY_NAME}
        INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${MORTON_ND_LIBRARY_NAME} INTERFACE cxx_std_14)

# This option allows Morton ND's tests to be built even if it's not the
# root project.
option(MORTON_ND_BUILD_TESTING "Build test library for ${PROJECT_NAME}" OFF)

# Include tests by default if this is the root project (done via CTest). Else, only builds if
# 'MORTON_ND_BUILD_TESTING' is ON.
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MORTON_ND_BUILD_TESTING) AND BUILD_TESTING)
    add_subdirectory(test)
endif()

# locations are provided by GNUInstallDirs
install(TARGETS ${MORTON_ND_LIBRARY_NAME} EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Create MortonNDConfigVersion.cmake for library consumers.
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

# Create MortonNDConfig.cmake from template for library consumers.
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Install MortonNDTargets.cmake
install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Install morton-nd-config.cmake and morton-nd-config-version.cmake
install(FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Install library headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/morton-nd/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h")

# Install documentation
install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR}
        FILES_MATCHING PATTERN "*.md")

# Install license and notice
install(FILES
        ${PROJECT_SOURCE_DIR}/LICENSE
        ${PROJECT_SOURCE_DIR}/NOTICE
        DESTINATION ${CMAKE_INSTALL_DOCDIR})