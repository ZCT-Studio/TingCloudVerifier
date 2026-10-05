# ── PostgreSQL (libpq) ──
# vcpkg libpq 没有 CMake Config, 只能用 CMake 自带 FindPostgreSQL.cmake (Module).
# GitHub Runner 预装了 C:\Program Files\PostgreSQL\XX\lib\libpq.lib (x64),
# find_package(PostgreSQL) 会先扫系统默认路径, x86 构建抢系统 x64 → LNK4272.
# 解决方案: 绕过 find_package, 手动 find_library / find_path 仅在 vcpkg 目录找.
set(TCV_HAS_PGSQL OFF)
if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(_pq_root "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

    find_path(PQ_INCLUDE_DIR
        NAMES libpq-fe.h postgres_ext.h
        PATHS "${_pq_root}/include"
        NO_DEFAULT_PATH)

    find_library(PQ_LIBRARY
        NAMES pq libpq
        PATHS "${_pq_root}/lib" "${_pq_root}/debug/lib"
        NO_DEFAULT_PATH)

    if(PQ_INCLUDE_DIR AND PQ_LIBRARY)
        if(NOT TARGET PostgreSQL::PostgreSQL)
            add_library(PostgreSQL::PostgreSQL UNKNOWN IMPORTED)
            set_target_properties(PostgreSQL::PostgreSQL PROPERTIES
                IMPORTED_LOCATION "${PQ_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${PQ_INCLUDE_DIR}")
        endif()
        message(STATUS "PostgreSQL (libpq) FOUND via vcpkg: ${PQ_LIBRARY}")
        set(TCV_HAS_PGSQL ON)
    else()
        message(STATUS "PostgreSQL (libpq) NOT found in vcpkg root=${_pq_root} — TCV without PostgreSQL")
    endif()
else()
    message(STATUS "PostgreSQL (libpq): VCPKG_INSTALLED_DIR not set — TCV without PostgreSQL")
endif()

# ── MySQL / MariaDB (libmariadb) ──
# libmariadb (MariaDB Connector/C) 纯客户端, Linux 不需要 libaio 等 server 依赖.
# API 100% 兼容 (头文件也是 <mysql.h>).
# vcpkg 提供 Config 模式: unofficial-libmariadb.
find_package(unofficial-libmariadb CONFIG QUIET)
if(TARGET unofficial::libmariadb::libmariadb)
    message(STATUS "MySQL (libmariadb) FOUND — enabling TCV_HAS_MYSQL")
    set(TCV_HAS_MYSQL ON)
else()
    message(STATUS "MySQL (libmariadb) NOT found — TCV will run WITHOUT MySQL support")
    set(TCV_HAS_MYSQL OFF)
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
