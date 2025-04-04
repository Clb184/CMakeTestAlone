# CMake version 3.5, idk why but it may be useful
cmake_minimum_required(VERSION 3.5)

# Project and name
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(CMAKE_TEST)

# Source files (cpp) and header files (hpp & h)
file(GLOB cpp_files src/*.cpp)
file(GLOB hpp_files src/*.hpp src/*.h)

# Get GLFW
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 43c9fb329116fcd57e94c8edc9ce2c1619a370a8
)

FetchContent_MakeAvailable(glfw)

# exe
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(GLEW_STATIC)

add_executable(${PROJECT_NAME} ${cpp_files})
add_subdirectory(glew/build/cmake)
target_link_libraries(${PROJECT_NAME} glfw glew_s)

# Additional include directories
target_include_directories(${PROJECT_NAME} 
	PUBLIC include/
	PUBLIC glew/include
	)

