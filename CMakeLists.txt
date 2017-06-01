project(bin2ea)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")

set(SOURCE_LIST
    main.cpp

    util/args.h
    util/args.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_LIST})
