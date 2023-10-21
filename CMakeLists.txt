cmake_minimum_required(VERSION 3.16.3)
project(asd C)

set(CMAKE_C_STANDARD 11)


add_executable(asd main.c
        requests.c
        requests.h
        parson.c
        parson.h
)

find_package(CURL REQUIRED)
target_link_libraries(asd CURL::libcurl)