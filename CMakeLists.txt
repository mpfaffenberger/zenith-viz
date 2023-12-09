cmake_minimum_required(VERSION 3.10)

project(zenith)

set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)


# headers are listed in a cmake file.
include_directories(zenith/cpp zenith/cpp/imgui /usr/local/include /usr/include /opt/homebrew/include)
link_directories(/usr/local/lib /usr/lib)
find_library(GLFW3 NAMES glfw glfw3 libGLFW libGLFW3 PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu/)
find_library(OPENGL NAMES opengl gl libopengl libOpenGL.so libOpenGL.dylib libOpenGL.dll libgl libGL PATHS /usr/local/lib /usr/lib /usr/local/lib/x86_64-linux-gnu)
find_path(GLM_INCLUDE_DIR glm/)
find_library(GLEW names GLEW glew libglew libGLEW PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu/)

message("glew stuff: "  ${GLEW} " " ${GLEW_INCLUDE_DIRS})
message("glm stuff: " ${GLM_INCLUDE_DIR})
message("glfw stuff: " ${GLFW3} " " ${glfw3_INCLUDE_DIRS})
message("OpenGL stuff: " ${OPENGL})
set(SOURCE ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/Controls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/Engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/GLBoilerPlate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/GLModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/main.cpp
)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/Controls.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/Engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/GLBoilerPlate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/GLModel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_impl_glfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_impl_opengl3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imgui_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/imstb_truetype.h
)
add_library(zenith SHARED ${SOURCE} ${HEADERS})
add_executable(main ${SOURCE} ${HEADERS})
target_link_libraries(zenith "-framework Cocoa")
target_link_libraries(zenith "-framework OpenGL")
target_link_libraries(zenith "-framework IOKit")
target_link_libraries(main "-framework Cocoa")
target_link_libraries(main "-framework OpenGL")
target_link_libraries(main "-framework IOKit")
target_link_libraries(zenith ${GLEW} ${GLFW3} ${OPENGL})
target_link_libraries(main ${GLEW} ${GLFW3} ${OPENGL})
install(TARGETS zenith LIBRARY DESTINATION zenith/lib)
