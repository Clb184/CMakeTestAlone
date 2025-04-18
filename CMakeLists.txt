# CMake version 3.5, idk why but it may be useful
cmake_minimum_required(VERSION 3.5)

# Project and name
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(CMAKE_TEST LANGUAGES C CXX ASM)

# Source files (cpp) and header files (hpp & h)
file(GLOB cpp_files src/*.cpp)
file(GLOB hpp_files src/*.hpp src/*.h)
file(GLOB asm_files src/*.asm) # Assembly for the win lol

# Some definitions for linking staticaly
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(GLEW_STATIC)
add_compile_definitions(__AVX__)

# Set some variables to disable stuff I don't want to build.
# I only use static libs so I have less files to deal with,
# and also use some debug postfix like d for debug builds
set(CMAKE_DEBUG_POSTFIX "d")
set(ZLIB_BUILD_STATIC ON)
set(ZLIB_BUILD_SHARED OFF)
set(PNG_STATIC ON)
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
set(PNG_TOOLS OFF)
set(PNG_EXECUTABLES OFF)
if(WIN32)
set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/deps/zlib/zs${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}" )
endif()
set(ZLIB_INCLUDE_DIR "${CMAKE_HOME_DIRECTORY}/deps/zlib/" )
set(ZLIB_ROOT "${CMAKE_HOME_DIRECTORY}/deps/zlib/" )

# Add the executable file
set(EXE_NAME ${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX})
add_executable(${EXE_NAME} ${cpp_files} ${asm_files})

# Libraries to link with

if(MSVC)
	message("sal.h already comes with VS...")
else()
	message("Getting sal.h for DirectXMath")
	execute_process(COMMAND curl -L --output ${CMAKE_HOME_DIRECTORY}/include/sal.h https://raw.githubusercontent.com/dotnet/runtime/main/src/coreclr/pal/inc/rt/sal.h)
endif()

add_subdirectory(deps/zlib/)
add_subdirectory(deps/libpng/)
add_subdirectory(deps/glew/build/cmake/)
add_subdirectory(deps/glfw/)
add_subdirectory(deps/DirectXMath)


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

target_link_libraries(${EXE_NAME} glfw glew_s png_static DirectXMath)

# Additional include directories
target_include_directories(${EXE_NAME} PUBLIC include/ PUBLIC deps/glew/include/ )
