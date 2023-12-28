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

# Try and find the Vulkan SDK
find_package(Vulkan REQUIRED)

# Download GLFW
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(glfw)

# Declare realtime library
add_library(realtime ${REALTIME_SOURCES})
target_include_directories(realtime PUBLIC ${CMAKE_SOURCE_DIR}/extern/ ${CMAKE_CURRENT_SOURCE_DIR}/ ${Vulkan_INCLUDE_DIRS})
target_link_libraries(realtime PUBLIC ${Vulkan_LIBRARIES} glfw)

# Disable CRT warnings and enable highest warning level
if (MSVC)
    target_compile_definitions(realtime PUBLIC _CRT_SECURE_NO_WARNINGS=1)
    target_compile_options(realtime PRIVATE /W4 /WX)
else ()
    target_compile_options(realtime PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

# Declare executable
add_executable(realtime-editor main.cc)
target_link_libraries(realtime-editor PUBLIC realtime)
