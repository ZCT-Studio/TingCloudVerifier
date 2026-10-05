# jsoncpp alias: Drogon 依赖它自己 FindJsoncpp.cmake 创建的 Jsoncpp_lib,
# 但 vcpkg jsoncppConfig.cmake 创建的是 jsoncpp_static / JsonCpp::JsonCpp.
# 用 ALIAS 完美保留 vcpkg target 的 Debug/Release IMPORTED_LOCATION.
if(TARGET jsoncpp_static AND NOT TARGET Jsoncpp_lib)
    add_library(Jsoncpp_lib ALIAS jsoncpp_static)
elseif(TARGET JsonCpp::JsonCpp AND NOT TARGET Jsoncpp_lib)
    add_library(Jsoncpp_lib ALIAS JsonCpp::JsonCpp)
endif()

find_package(Drogon                 CONFIG REQUIRED)
find_package(unofficial-sqlite3     CONFIG REQUIRED)
find_package(yaml-cpp               CONFIG REQUIRED)
find_package(OpenSSL                REQUIRED)
find_package(unofficial-argon2      CONFIG REQUIRED)

# vcpkg triplet 下 IMPORTED_LOCATION 可能在不同位置, 统一尝试所有属性
foreach(_tgt unofficial::sqlite3::sqlite3 unofficial::argon2::libargon2)
    if(TARGET )
        get_target_property(_loc  IMPORTED_LOCATION_RELEASE)
        if(NOT _loc)
            get_target_property(_loc  IMPORTED_LOCATION)
            if(NOT _loc)
                get_target_property(_loc  IMPORTED_IMPLIB_RELEASE)
                if(NOT _loc)
                    get_target_property(_loc  IMPORTED_IMPLIB)
                endif()
            endif()
        endif()
        if(NOT _loc OR NOT EXISTS "")
            message(FATAL_ERROR
                "vcpkg import target  has no resolvable library file.\n"
                "  IMPORTED_LOCATION_RELEASE=''\n"
                "This usually means you're using a broken vcpkg triplet.\n"
                "Try switching to a static triplet (e.g. arm64-osx-static, x64-osx-static).")
        endif()
        message(STATUS "Resolved  -> ")
    endif()
endforeach()
