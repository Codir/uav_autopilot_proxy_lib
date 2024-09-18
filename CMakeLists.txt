cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(autopilot)

find_package(MAVSDK REQUIRED)

add_executable(autopilot
    main.cpp
    #mavsdk_interface.cpp
    #mavsdk_interface.h
)

target_link_libraries(autopilot
    MAVSDK::mavsdk
)
