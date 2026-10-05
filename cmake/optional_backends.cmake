# ── PostgreSQL (libpq) ──
# vcpkg libpq port 不提供 CMake Config, 提供 share/postgresql/vcpkg-cmake-wrapper.cmake
# 配合 CMake 自带 FindPostgreSQL.cmake (Module 模式), 所以 find_package 名是 PostgreSQL.
# wrapper 用了 NO_DEFAULT_PATH, 不会抢系统预装的 PostgreSQL.
find_package(PostgreSQL QUIET)
if(TARGET PostgreSQL::PostgreSQL)
    message(STATUS "PostgreSQL (libpq via vcpkg) FOUND — enabling TCV_HAS_PGSQL")
    set(TCV_HAS_PGSQL ON)
else()
    message(STATUS "PostgreSQL (libpq) NOT found — TCV will run WITHOUT PostgreSQL support")
    set(TCV_HAS_PGSQL OFF)
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
