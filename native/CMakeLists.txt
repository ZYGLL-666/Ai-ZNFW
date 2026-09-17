cmake_minimum_required(VERSION 3.10)
project(ai_service CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ai_service
    src/main.cpp
    src/common.cpp
    src/logger.cpp
    src/clean.cpp
    src/suppress.cpp
    src/service.cpp
    src/action.cpp
    src/boot.cpp
)

target_include_directories(ai_service PRIVATE src)

if(ANDROID)
    target_link_libraries(ai_service PRIVATE android log)
endif()
