//
// MIT License
//
// Copyright (c) 2024 Elias Engelbert Plank
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef REALTIME_WINDOW_H
#define REALTIME_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

#include "event.h"
#include "realtime.h"

namespace rt {

/// A window acts as a draw surface for the realtime engine and is also
/// responsible for handling all user input.
class Window {
public:
    constexpr static auto VERTICAL_SYNC = false;

    /// The window specification tells the window the desired size and the
    /// title.
    struct Specification {
        s32 width = 800;
        s32 height = 600;
        std::string name = "Real-time rendering";
    };

    /// Creates a window using the provided specification.
    /// @param specification The specification
    explicit Window(Specification specification);

    /// Destroys the window and its draw surface
    ~Window();

    /// A window cannot be copied or moved
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    /// Indicates whether the window should close or not
    /// @return A value that indicates whether the window should close
    bool should_close() const;

    /// Indicates whether the window is resized or not
    /// @return A value that indicates whether the window is resized or not
    bool is_window_resized() const;

    /// Clears the window resize flag
    void clear_window_resized();

    /// Creates a surface for the specified Vulkan instance
    /// @param instance The Vulkan instance
    /// @param surface The surface
    void create_surface(VkInstance instance, VkSurfaceKHR *surface) const;

    /// Retrieves the extent of the window
    /// @return The extent of the window
    VkExtent2D extent() const;

    /// Retrieves the GLFW native window handle
    /// @return The GLFW native window handle
    GLFWwindow *native_handle() const;

    /// Registers an event listener
    /// @param listener The event listener
    void register_listener(const EventListener &listener);

private:
    /// Callback for when the framebuffer resizes
    /// @param window The GLFW window handle
    /// @param width The new width of the framebuffer
    /// @param height The new height of the framebuffer
    static void framebuffer_resize_callback(GLFWwindow *window, s32 width, s32 height);

    /// Callback for when the mouse scrolls
    /// @param window The GLFW window handle
    /// @param x The scroll in the x direction
    /// @param y The scroll in the y direction
    static void scroll_callback(GLFWwindow *window, f64 x, f64 y);

    /// Callback for when the cursor moves
    /// @param window The GLFW window handle
    /// @param x The movement in the x direction
    /// @param y The movement in the y direction
    static void cursor_callback(GLFWwindow *window, f64 x, f64 y);

    /// Dispatches an event to all listeners
    /// @param event The event
    void dispatch(const Event &event);

    /// Our list of friends :)
    friend class Device;
    friend class Application;

    bool framebuffer_resized;
    Specification spec;
    GLFWwindow *window;
    std::vector<EventListener> listeners;
};

}// namespace rt

#endif// REALTIME_WINDOW_H
