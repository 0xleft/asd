cmake_minimum_required(VERSION 3.16.3)
project(asd C)

set(CMAKE_C_STANDARD 11)


add_executable(asd main.c
        packages.c
        packages.h
        thirdparty/parson.c
        thirdparty/parson.h
        utils.c utils.h
        caching.c
        caching.h)

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(asd CURL::libcurl)
target_link_libraries(asd pthread)