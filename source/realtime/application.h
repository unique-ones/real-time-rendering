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

#ifndef REALTIME_APPLICATION_H
#define REALTIME_APPLICATION_H

#include "device.h"
#include "pipeline.h"
#include "window.h"

namespace rt {

    class Application {
    public:
        /// Alias for window specification
        using Specification = Window::Specification;

        /// Creates a realtime application
        /// @param specification The application specification
        explicit Application(Specification specification);

        /// An application cannot be copied or moved
        Application(const Application &w) = delete;
        Application &operator=(const Application &w) = delete;
        Application(Application &&w) = delete;
        Application &operator=(Application &&w) = delete;

        /// Runs the application
        void run();

    private:
        Window window;
        Device device;
        Pipeline pipeline;
    };
}// namespace rt

#endif// REALTIME_APPLICATION_H
