add_executable(TingCloudVerifier
    main.cpp
)

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(TingCloudVerifier
    PRIVATE
    Drogon::Drogon
)