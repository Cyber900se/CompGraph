cmake_minimum_required(VERSION 3.31)
project(CG_Lab4)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

add_executable(CG_Lab4 WIN32 WinMain.cpp
        Framework/Application.cpp
        Framework/Application.h
        Framework/Window.cpp
        Framework/Window.h
        Framework/Graphics.cpp
        Framework/Graphics.h
        Framework/Renderer.cpp
        Framework/Renderer.h
        Framework/InputHandler.cpp
        Framework/InputHandler.h)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(LIBS
        gdi32
        d3d11
        d3dcompiler
        dxguid)

target_link_libraries(CG_Lab4 ${LIBS})