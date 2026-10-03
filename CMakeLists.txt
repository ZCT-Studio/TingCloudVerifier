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

# Verify vcpkg import targets actually resolve to library files.
# macOS dynamic triplets are known to produce broken IMPORTED_LOCATION for
# unofficial-sqlite3 (empty / wrong path), which silently yields a link line
# without the library and -> "symbol(s) not found" at link time.
foreach(_tgt unofficial::sqlite3::sqlite3 unofficial::argon2::libargon2)
    if(TARGET ${_tgt})
        get_target_property(_loc ${_tgt} IMPORTED_LOCATION_RELEASE)
        if(NOT _loc)
            get_target_property(_loc ${_tgt} IMPORTED_LOCATION)
            if(NOT _loc)
                get_target_property(_loc ${_tgt} IMPORTED_IMPLIB_RELEASE)
                if(NOT _loc)
                    get_target_property(_loc ${_tgt} IMPORTED_IMPLIB)
                endif()
            endif()
        endif()
        if(NOT _loc OR NOT EXISTS "${_loc}")
            message(FATAL_ERROR
                "vcpkg import target ${_tgt} has no resolvable library file.\n"
                "  IMPORTED_LOCATION_RELEASE='${_loc}'\n"
                "This usually means you're using a broken vcpkg triplet.\n"
                "Try switching to a static triplet (e.g. arm64-osx-static, x64-osx-static).")
        endif()
        message(STATUS "Resolved ${_tgt} -> ${_loc}")
    endif()
endforeach()

# ── 可选后端 ──
# PostgreSQL: 运行时支持, 需要 vcpkg install libpq
find_package(PostgreSQL QUIET)
if(PostgreSQL_FOUND)
    message(STATUS "PostgreSQL (libpq) FOUND — enabling TCV_HAS_PGSQL")
    set(TCV_HAS_PGSQL ON)
else()
    message(STATUS "PostgreSQL (libpq) NOT found — TCV will run WITHOUT PostgreSQL support")
    set(TCV_HAS_PGSQL OFF)
endif()

# MySQL/MariaDB: 运行时支持, 需要 vcpkg install libmysql
find_package(unofficial-libmysql CONFIG QUIET)
if(TARGET unofficial::libmysql::libmysql)
    message(STATUS "MySQL (libmysql) FOUND — enabling TCV_HAS_MYSQL")
    set(TCV_HAS_MYSQL ON)
else()
    message(STATUS "MySQL (libmysql) NOT found — TCV will run WITHOUT MySQL support")
    set(TCV_HAS_MYSQL OFF)
endif()

target_link_libraries(TingCloudVerifier PRIVATE
    Drogon::Drogon
    unofficial::sqlite3::sqlite3
    yaml-cpp::yaml-cpp
    OpenSSL::SSL OpenSSL::Crypto
    unofficial::argon2::libargon2
)

if(TCV_HAS_PGSQL)
    target_link_libraries(TingCloudVerifier PRIVATE PostgreSQL::PostgreSQL)
    target_compile_definitions(TingCloudVerifier PRIVATE TCV_HAS_PGSQL=1)
endif()

if(TCV_HAS_MYSQL)
    target_link_libraries(TingCloudVerifier PRIVATE unofficial::libmysql::libmysql)
    target_compile_definitions(TingCloudVerifier PRIVATE TCV_HAS_MYSQL=1)
endif()

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

