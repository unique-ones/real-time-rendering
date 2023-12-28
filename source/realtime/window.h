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

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <string>

#include "realtime.h"

namespace rt {

    /// A window acts as a draw surface for the realtime engine and is also
    /// responsible for handling all user input.
    class Window {
    public:
        /// The window specification tells the window the desired size and the
        /// title.
        struct Specification {
            s32 width = 1280;
            s32 height = 720;
            std::string name = "Real-time rendering";
        };

        /// Creates a window using the provided specification.
        /// @param specification The specification
        explicit Window(Specification specification);

        /// Destroys the window and its draw surface
        ~Window();

        /// A window cannot be copied or moved
        Window(const Window &w) = delete;
        Window &operator=(const Window &w) = delete;
        Window(Window &&w) = delete;
        Window &operator=(Window &&w) = delete;

        /// Indicates whether the window should close or not
        /// @return A value that indicates whether the window should close
        bool should_close() const;

    private:
        Specification spec;
        GLFWwindow *window;
    };
}// namespace rt

#endif// REALTIME_WINDOW_H
