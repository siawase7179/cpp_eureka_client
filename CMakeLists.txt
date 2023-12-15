cmake_minimum_required(VERSION 3.10)
project(eureka_cpp)

add_compile_options(-g -Wall -Og -O3 -std=c++11 -Wno-reorder -fpermissive -DIB_DEBUG_3 )

include_directories(
    ./
    
)

find_package(RapidJSON)

link_libraries(
    curl
)

add_executable(
    eureka_client
    eureka_example.cpp
    eureka_http.cpp
    eureka_client.cpp
)


install(
    TARGETS eureka_client 
    DESTINATION ${PROJECT_SOURCE_DIR}/bin/
)