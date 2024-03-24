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
Window::Window(Specification specification) : framebuffer_resized(false), spec(std::move(specification)) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(spec.width, spec.height, spec.name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
}

/// Destroys the window and its draw surface
Window::~Window() {
    glfwDestroyWindow(window);
}

/// Indicates whether the window should close or not
bool Window::should_close() const {
    return glfwWindowShouldClose(window);
}

/// Indicates whether the window is resized or not
bool Window::is_window_resized() const {
    return framebuffer_resized;
}

/// Clears the window resize flag
void Window::clear_window_resized() {
    framebuffer_resized = false;
}

/// Creates a surface for the specified Vulkan instance
void Window::create_surface(VkInstance instance, VkSurfaceKHR *surface) const {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        error(64, "[device] Failed to create window surface!");
    }
}

/// Retrieves the extent of the window
VkExtent2D Window::extent() const {
    return { static_cast<u32>(spec.width), static_cast<u32>(spec.height) };
}

/// Retrieves the GLFW native window handle
GLFWwindow *Window::native_handle() const {
    return window;
}

/// Callback for when the framebuffer resizes
void Window::framebuffer_resize_callback(GLFWwindow *window, s32 width, s32 height) {
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    win->framebuffer_resized = true;
    win->spec.width = width;
    win->spec.height = height;
}

}// namespace rt
