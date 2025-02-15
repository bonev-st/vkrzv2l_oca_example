cmake_minimum_required(VERSION 3.16)
project(src)

set(CMAKE_CXX_STANDARD 20)

add_executable(oca_sample OCA_sample.cpp)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(oca_sample
        ${OpenCV_LIBS}
        pthread
)
