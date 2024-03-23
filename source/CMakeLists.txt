#
# MIT License
#
# Copyright (c) 2023 Elias Engelbert Plank
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Gather sources
file(GLOB REALTIME_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/realtime/*.cc)

file(GLOB REALTIME_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/realtime/*.h)

# Try and find the Vulkan SDK
find_package(Vulkan REQUIRED)

# Download GLFW
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(glfw)

# Declare realtime library
add_library(realtime ${REALTIME_SOURCES} ${REALTIME_HEADERS})
target_include_directories(realtime PUBLIC ${CMAKE_SOURCE_DIR}/extern/ ${CMAKE_CURRENT_SOURCE_DIR}/ ${Vulkan_INCLUDE_DIRS})
target_link_libraries(realtime PUBLIC ${Vulkan_LIBRARIES} glfw)

# Disable CRT warnings and enable highest warning level
if (MSVC)
    target_compile_definitions(realtime PUBLIC _CRT_SECURE_NO_WARNINGS=1)
    target_compile_options(realtime PRIVATE /W4 /WX)
else ()
    target_compile_options(realtime PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-packed-bitfield-compat)
endif ()

# Find all vertex and fragment sources within shaders directory
find_program(GLSL_VALIDATOR glslangValidator HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)

# Get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_INSTALL_PREFIX}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        realtime-shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

# Declare executable
add_executable(realtime-editor main.cc)
target_link_libraries(realtime-editor PUBLIC realtime)
add_dependencies(realtime-editor realtime-shaders)
