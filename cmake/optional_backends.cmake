# ── PostgreSQL (libpq) ──
# vcpkg 重写了 find_package (toolchain.cmake L788): 调 find_package(PostgreSQL) 时,
#   先检查 <triplet>/share/postgresql/vcpkg-cmake-wrapper.cmake 是否存在.
#   存在 → include 它 (wrapper 用 NO_DEFAULT_PATH 只搜 vcpkg 目录, 再调原生 FindPostgreSQL).
#   不存在 → 直接 _find_package(PostgreSQL) 扫系统默认路径 —— GitHub Runner 上会扫到
#            C:\Program Files\PostgreSQL\14\lib\libpq.lib (x64), 给 x86 target 链接即 LNK4272.
#
# 所以关键约束: 只有 vcpkg 确实装了 libpq (wrapper 存在), 才调 find_package(PostgreSQL).
# 否则跳过, 别让原生 FindPostgreSQL 去碰系统预装的 PostgreSQL.
set(TCV_HAS_PGSQL OFF)

if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(_pq_wrapper "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/postgresql/vcpkg-cmake-wrapper.cmake")
    if(EXISTS "${_pq_wrapper}")
        message(STATUS "PostgreSQL wrapper present (vcpkg libpq installed), finding...")
        find_package(PostgreSQL QUIET)
        if(TARGET PostgreSQL::PostgreSQL)
            message(STATUS "PostgreSQL (libpq) FOUND — enabling TCV_HAS_PGSQL")
            set(TCV_HAS_PGSQL ON)
        else()
            message(STATUS "PostgreSQL (libpq) NOT found despite wrapper — TCV without PostgreSQL")
        endif()
    else()
        message(STATUS "PostgreSQL wrapper NOT present — skipping find_package (vcpkg libpq not installed)")
    endif()
else()
    message(STATUS "PostgreSQL: VCPKG_INSTALLED_DIR not set — skipping")
endif()

# ── MySQL / MariaDB (libmariadb) ──
# libmariadb 导出 CMake Config (share/unofficial-libmariadb/unofficial-libmariadb-config.cmake).
# 同样, 只有 share 目录存在时才调 find_package, 否则 GitHub Runner 上可能意外链接系统预装.
set(TCV_HAS_MYSQL OFF)

if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(_ma_share "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/unofficial-libmariadb")
    if(EXISTS "${_ma_share}")
        message(STATUS "MySQL share present (vcpkg libmariadb installed), finding...")
        find_package(unofficial-libmariadb CONFIG QUIET)
        if(TARGET unofficial::libmariadb::libmariadb)
            message(STATUS "MySQL (libmariadb) FOUND — enabling TCV_HAS_MYSQL")
            set(TCV_HAS_MYSQL ON)
        else()
            message(STATUS "MySQL (libmariadb) NOT found despite share — TCV without MySQL")
        endif()
    else()
        message(STATUS "MySQL share NOT present — skipping find_package (vcpkg libmariadb not installed)")
    endif()
else()
    message(STATUS "MySQL: VCPKG_INSTALLED_DIR not set — skipping")
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
