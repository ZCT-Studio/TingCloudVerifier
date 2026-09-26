cmake_minimum_required(VERSION 3.25)

project(TingCloudVerifier
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(NOMINMAX)
endif()

set(TCV_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs"
)

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

if(WIN32)
    set_target_properties(TingCloudVerifier PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
