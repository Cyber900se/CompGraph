cmake_minimum_required(VERSION 3.30)
project(CG_Lab1)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

include_directories(.)

add_executable(CG_Lab1
    MySuper3DApp.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(LIBS
        gdi32
        d3d11
        d3dcompiler
        dxguid)

target_link_libraries(CG_Lab1 ${LIBS})
