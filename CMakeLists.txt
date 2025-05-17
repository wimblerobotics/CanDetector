cmake_minimum_required(VERSION 3.10)
project(CanDetector VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(CanDetector src/main.cpp src/annotation.cpp src/descriptor.cpp)

target_include_directories(CanDetector PRIVATE ${OpenCV_INCLUDE_DIRS} include)
target_link_libraries(CanDetector PRIVATE ${OpenCV_LIBS})

# Installation
install(TARGETS CanDetector RUNTIME DESTINATION bin)
