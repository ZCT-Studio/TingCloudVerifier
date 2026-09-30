cmake_minimum_required(VERSION 3.25)

project(TingCloudVerifier
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(NOMINMAX)
    # Drogon/trantor/yaml-cpp from vcpkg are built with static CRT (MT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(TCV_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs"
)

# Cross-compilation: Drogon's FindFilesystem uses try_run which can't
# execute cross-compiled binaries. Pre-set all three tests.
# See: drogon FindFilesystem.cmake (vcpkg_installed/*/share/drogon/)
if(CMAKE_CROSSCOMPILING)
    if(MSVC)
        set(CXX_FILESYSTEM_NO_LINK_NEEDED TRUE  CACHE INTERNAL "")
        set(CXX_FILESYSTEM_STDCPPFS_NEEDED FALSE CACHE INTERNAL "")
        set(CXX_FILESYSTEM_CPPFS_NEEDED      FALSE CACHE INTERNAL "")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
            set(CXX_FILESYSTEM_NO_LINK_NEEDED FALSE CACHE INTERNAL "")
            set(CXX_FILESYSTEM_STDCPPFS_NEEDED TRUE  CACHE INTERNAL "")
            set(CXX_FILESYSTEM_CPPFS_NEEDED      FALSE CACHE INTERNAL "")
        else()
            set(CXX_FILESYSTEM_NO_LINK_NEEDED FALSE CACHE INTERNAL "")
            set(CXX_FILESYSTEM_STDCPPFS_NEEDED FALSE CACHE INTERNAL "")
            set(CXX_FILESYSTEM_CPPFS_NEEDED      TRUE  CACHE INTERNAL "")
        endif()
    else()
        # Clang / other: assume cppfs link needed
        set(CXX_FILESYSTEM_NO_LINK_NEEDED FALSE CACHE INTERNAL "")
        set(CXX_FILESYSTEM_STDCPPFS_NEEDED FALSE CACHE INTERNAL "")
        set(CXX_FILESYSTEM_CPPFS_NEEDED      TRUE  CACHE INTERNAL "")
    endif()
endif()

find_package(Drogon                 CONFIG REQUIRED)
find_package(unofficial-sqlite3     CONFIG REQUIRED)
find_package(yaml-cpp               CONFIG REQUIRED)
find_package(OpenSSL                REQUIRED)
find_package(unofficial-argon2      CONFIG REQUIRED)

add_executable(TingCloudVerifier src/main.cpp)

target_include_directories(TingCloudVerifier PRIVATE ${TCV_INCLUDE_DIRS})

target_link_libraries(TingCloudVerifier PRIVATE
    Drogon::Drogon
    unofficial::sqlite3::sqlite3
    yaml-cpp::yaml-cpp
    OpenSSL::SSL OpenSSL::Crypto
    unofficial::argon2::libargon2
)

target_compile_definitions(TingCloudVerifier PRIVATE TCV_HAS_ARGON2=1)

# GCC < 12 requires -lstdc++fs for std::filesystem (used by Drogon)
if(CMAKE_CROSSCOMPILING AND NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
        target_link_libraries(TingCloudVerifier PRIVATE stdc++fs)
    endif()
endif()

if(WIN32)
    set_target_properties(TingCloudVerifier PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
