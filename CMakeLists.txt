project(bin2ea)
cmake_minimum_required(VERSION 3.1)

# Setting C++ Standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SOURCE_LIST
    main.cpp

	lib/hex_convert.h
	lib/byte_pack.h

    util/args.h
    util/args.cpp
)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
add_executable(${PROJECT_NAME} ${SOURCE_LIST})
