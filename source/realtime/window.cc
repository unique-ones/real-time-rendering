//
// MIT License
//
// Copyright (c) 2023 Elias Engelbert Plank
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "window.h"
#include "utility.h"

#include <type_traits>

namespace rt {

    /// Creates a window using the provided specification.
    Window::Window(Specification specification) : spec(std::move(specification)) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(spec.width, spec.height, spec.name.c_str(), nullptr, nullptr);
    }

    /// Destroys the window and its draw surface
    Window::~Window() {
        glfwDestroyWindow(window);
    }

    /// Indicates whether the window should close or not
    bool Window::should_close() const {
        return glfwWindowShouldClose(window);
    }

    /// Creates a surface for the specified Vulkan instance
    void Window::create_surface(VkInstance instance, VkSurfaceKHR *surface) const {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            error(64, "[device] Failed to create window surface!");
        }
    }

}// namespace rt
