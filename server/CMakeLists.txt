cmake_minimum_required(VERSION 3.0.0)
project(ImageFlipServer VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
find_package(Boost COMPONENTS thread system REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories(${Boost_INCLUDE_DIR})

add_executable(${PROJECT_NAME} main.cpp ) 

add_library(HttpConnection STATIC http_connection.cpp)

add_library(Server STATIC server.cpp)

target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES} Server HttpConnection)