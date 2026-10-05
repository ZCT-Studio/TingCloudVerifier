# ── PostgreSQL (libpq) ──
# vcpkg 重写了 find_package (toolchain.cmake L788), 调 find_package(PostgreSQL) 时会自动 include
#   share/postgresql/vcpkg-cmake-wrapper.cmake
# wrapper 先 find_library(PostgreSQL_LIBRARY_RELEASE NAMES pq libpq PATHS vcpkg_root/lib NO_DEFAULT_PATH REQUIRED)
# 再 _find_package(PostgreSQL) —— 原生 FindPostgreSQL.cmake 发现变量已预填, 不会扫系统路径.
# 所以 find_package(PostgreSQL) 在 Windows(不会抢 C:\Program Files) / Linux / macOS 都安全.
set(TCV_HAS_PGSQL OFF)

# 先检查 vcpkg 是否真的装了 libpq (wrapper 是否存在)
if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(_pq_wrapper "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/postgresql/vcpkg-cmake-wrapper.cmake")
    if(EXISTS "${_pq_wrapper}")
        message(STATUS "PostgreSQL wrapper found: ${_pq_wrapper}")
    else()
        message(STATUS "PostgreSQL wrapper NOT found at ${_pq_wrapper} — vcpkg probably didn't install libpq")
    endif()
else()
    message(STATUS "PostgreSQL: VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}, VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}")
endif()

find_package(PostgreSQL QUIET)
if(TARGET PostgreSQL::PostgreSQL)
    message(STATUS "PostgreSQL (libpq) FOUND via find_package(PostgreSQL) — enabling TCV_HAS_PGSQL")
    set(TCV_HAS_PGSQL ON)
else()
    message(STATUS "PostgreSQL (libpq) NOT found — TCV will run WITHOUT PostgreSQL support")
    if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
        set(_pq_root "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
        message(STATUS "  vcpkg root = ${_pq_root}")
        if(EXISTS "${_pq_root}/include/libpq-fe.h")
            message(STATUS "  include EXISTS but library not found — check .so symlinks")
        else()
            message(STATUS "  include NOT FOUND — libpq port may not have been installed")
        endif()
    endif()
endif()

# ── MySQL / MariaDB (libmariadb) ──
# libmariadb port 自带 CMake Config (share/unofficial-libmariadb/):
#   portfile.cmake: vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-libmariadb)
#   target 名: unofficial::libmariadb::libmariadb
set(TCV_HAS_MYSQL OFF)
find_package(unofficial-libmariadb CONFIG QUIET)
if(TARGET unofficial::libmariadb::libmariadb)
    message(STATUS "MySQL (libmariadb) FOUND via find_package(unofficial-libmariadb CONFIG) — enabling TCV_HAS_MYSQL")
    set(TCV_HAS_MYSQL ON)
else()
    message(STATUS "MySQL (libmariadb) NOT found — TCV will run WITHOUT MySQL support")
    if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
        set(_ma_share "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/unofficial-libmariadb")
        if(EXISTS "${_ma_share}")
            message(STATUS "  share EXISTS: ${_ma_share}")
            message(STATUS "  but target unofficial::libmariadb::libmariadb not created — check Config file")
        else()
            message(STATUS "  share NOT FOUND at ${_ma_share} — vcpkg probably didn't install libmariadb")
        endif()
    endif()
endif()

# ── 条件链接 ──
if(TCV_HAS_PGSQL)
    target_link_libraries(TingCloudVerifier PRIVATE PostgreSQL::PostgreSQL)
    target_compile_definitions(TingCloudVerifier PRIVATE TCV_HAS_PGSQL=1)
endif()

if(TCV_HAS_MYSQL)
    target_link_libraries(TingCloudVerifier PRIVATE unofficial::libmariadb::libmariadb)
    target_compile_definitions(TingCloudVerifier PRIVATE TCV_HAS_MYSQL=1)
endif()
