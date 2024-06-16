cmake_minimum_required(VERSION 3.10)

project(zenith)

set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)


macro(print_list LIST_NAME MESSAGE)
    foreach(item IN ITEMS ${${LIST_NAME}})
        message(${MESSAGE} ${item})
    endforeach()
endmacro()

macro(glob_headers source_dir output_var)
    FILE(GLOB ${output_var} CONFIGURE_DEPENDS ${source_dir}/*.hpp ${source_dir}/*.h})
    print_list(${output_var} "Found Header File: ")
endmacro()

macro(glob_sources source_dir output_var)
    FILE(GLOB ${output_var} CONFIGURE_DEPENDS ${source_dir}/*.c ${source_dir}/*.cpp)
    print_list(${output_var} "Found Source File: ")
endmacro()

glob_sources(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp ZENITH_SOURCES)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp ZENITH_HEADERS)

glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glm/glm GLM_BASE)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glm/glm/detail GLM_DETAIL)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glm/glm/ext GLM_EXT)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glm/glm/gtc GLM_GTC)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glm/glm/gtx GLM_GTX)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glm/glm/simd GLM_SIMD)

set(GLM_ALL ${GLM_BASE} ${GLM_DETAIL} ${GLM_EXT} ${GLM_GTC} ${GLM_GTX} ${GLM_SIMD})

glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glfw/include/GLFW GLFW_BASE)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glfw/src GLFW_SRC_HEADERS)
glob_sources(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glfw/src GLFW_SRC_FILES)

set(GLFW_ALL ${GLFW_BASE} ${GLFW_SRC_HEADERS} ${GLFW_SRC_FILES})

glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glad/include/glad GLAD_BASE)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glad/include/KHR GLAD_KHR)
glob_sources(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/glad/src GLAD_SRC)

set(GLAD_ALL ${GLAD_BASE} ${GLAD_KHR} ${GLAD_SRC})

glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui IMGUI_BASE_HEADERS)
glob_sources(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui IMGUI_BASE_SOURCES)

set(IMGUI_ALL ${IMGUI_BASE_HEADERS} ${IMGUI_BASE_SOURCES})
LIST(
    APPEND
    IMGUI_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/backends/imgui_impl_opengl3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/backends/imgui_impl_opengl3_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/imgui/backends/imgui_impl_opengl3.cpp
)

glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/pybind11/include/pybind11 PYBIND11_BASE)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/pybind11/include/pybind11/detail PYBIND11_DETAIL)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/pybind11/include/pybind11/stl PYBIND11_STL)
glob_headers(${CMAKE_CURRENT_SOURCE_DIR}/zenith/cpp/pybind11/include/pybind11/eigen PYBIND11_EIGEN)

set(PYBIND_ALL, ${PYBIND11_BASE} ${PYBIND11_DETAIL} ${PYBIND11_STL} ${PYBIND11_EIGEN})

set(ALL_SOURCES ${ZENITH_SOURCES} ${ZENITH_HEADERS} ${GLAD_ALL} ${GLFW_ALL} ${GLM_ALL} ${IMGUI_ALL} ${PYBIND_ALL})

execute_process(
    COMMAND which python
    OUTPUT_VARIABLE PYTHON_EXEC
    ERROR_VARIABLE ERROR_MSG
)

execute_process(
    COMMAND python -c "import sys; print(sys.version[:4])"
    OUTPUT_VARIABLE PY_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

cmake_path(GET PYTHON_EXEC PARENT_PATH PYTHON_DIR)
cmake_path(GET PYTHON_DIR PARENT_PATH PYTHON_DIR)
set(PYTHON_INCLUDE_DIR ${PYTHON_DIR}/include/python${PY_VERSION})
message("py_include: " ${PYTHON_INCLUDE_DIR})
include_directories(zenith/cpp zenith/cpp/imgui zenith/cpp/glm zenith/cpp/glfw/include zenith/cpp/pybind11/include zenith/cpp/glad/include)
set(PYTHON_DOT_H "${PYTHON_INCLUDE_DIR}/Python.h")
message("Python: " ${PYTHON_DOT_H})
include_directories(${PYTHON_INCLUDE_DIR})
add_library(zenith SHARED ${ALL_SOURCES})
target_link_libraries(zenith ${PYTHON_LIBRARIES})
install(TARGETS zenith LIBRARY DESTINATION zenith/lib)
add_executable(main ${ALL_SOURCES})
