find_package(Drogon                 CONFIG REQUIRED)
find_package(unofficial-sqlite3     CONFIG REQUIRED)
find_package(yaml-cpp               CONFIG REQUIRED)
find_package(OpenSSL                REQUIRED)
find_package(unofficial-argon2      CONFIG REQUIRED)

# ── jsoncpp Debug/Release 路径修复 ──
# Drogon 自带的 FindJsoncpp.cmake 用 find_library(NAMES jsoncpp) 不区分 config,
# 它本来有的 debug/optimized 区分代码被注释掉了.
# 覆盖它创建的 Jsoncpp_lib INTERFACE target, 用 generator expression 区分路径.
# 先清掉 FindJsoncpp.cmake 设置的单一路径, 再加回 config-sensitive 的.
if(TARGET Jsoncpp_lib AND VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(_jsoncpp_debug "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib/jsoncpp.lib")
    set(_jsoncpp_release "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/jsoncpp.lib")
    # 清掉旧的, 用 target_link_libraries 加新的 (接受 generator expression)
    set_target_properties(Jsoncpp_lib PROPERTIES INTERFACE_LINK_LIBRARIES "")
    target_link_libraries(Jsoncpp_lib INTERFACE
        "$<$<CONFIG:Debug>:${_jsoncpp_debug}>"
        "$<$<NOT:$<CONFIG:Debug>>:${_jsoncpp_release}>"
    )
    message(STATUS "Patched Jsoncpp_lib: Debug→${_jsoncpp_debug}, Release→${_jsoncpp_release}")
endif()

# vcpkg triplet 下 IMPORTED_LOCATION 可能在不同位置, 统一尝试所有属性
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
