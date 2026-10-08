find_package(Drogon                 CONFIG REQUIRED)
find_package(unofficial-sqlite3     CONFIG REQUIRED)
find_package(yaml-cpp               CONFIG REQUIRED)
find_package(OpenSSL                REQUIRED)
find_package(unofficial-argon2      CONFIG REQUIRED)

# ── jsoncpp: 修正 Debug/Release 混合链接 ──
# Drogon 自带 FindJsoncpp.cmake (Module) 只用 find_library 找一个库,
# 不区分 config. vcpkg static triplet 下 Debug 版在 <triplet>/debug/lib/,
# Release 在 <triplet>/lib/, Drogon 会找 Release 版 → Debug 构建 CRT 混链.
# 非 static triplet (x64-linux, arm64-osx 等动态 triplet), 所有 config
# 共用 <triplet>/lib/, 不需要 debug 分支.
# 修复: 只有 debug/lib/ 存在时才启用 generator expression, 否则保持 Drogon 原生结果.
if(TARGET Jsoncpp_lib AND VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(_icd_root "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    if(WIN32)
        set(_jl_suffix ".lib")
    else()
        set(_jl_suffix ".a")
    endif()

    set(_jl_release "${_icd_root}/lib/jsoncpp${_jl_suffix}")
    set(_jl_debug   "${_icd_root}/debug/lib/jsoncpp${_jl_suffix}")

    if(EXISTS "${_jl_debug}")
        set_target_properties(Jsoncpp_lib PROPERTIES INTERFACE_LINK_LIBRARIES "")
        target_link_libraries(Jsoncpp_lib INTERFACE
            "$<$<CONFIG:Debug>:${_jl_debug}>"
            "$<$<NOT:$<CONFIG:Debug>>:${_jl_release}>"
        )
        message(STATUS "Patched Jsoncpp_lib (static triplet): Debug=${_jl_debug}, Release=${_jl_release}")
    else()
        message(STATUS "Jsoncpp_lib: no debug/lib (non-static triplet), keeping Drogon native result")
    endif()
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
