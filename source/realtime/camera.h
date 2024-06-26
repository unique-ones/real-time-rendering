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

#ifndef REALTIME_CAMERA_H
#define REALTIME_CAMERA_H

#include "realtime.h"
#include "window.h"

namespace rt {

struct Camera {
    Window &window;
    f32 azimuth;
    f32 declination;
    f32 distance;
    glm::vec3 target;
    glm::mat4 projection;
    glm::mat4 view;

    // Movement state
    bool clicked{};
    glm::vec2 cursor_start{};
    glm::vec2 cursor_current{};

    /// Creates a new orbital camera
    /// @param position The initial position of the camera
    /// @param target The target of the camera
    explicit Camera(Window &window, const glm::vec3 &target = { 0, 0, 0 });

    /// Updates the camera
    /// @param aspect The aspect ratio
    void update(f32 aspect);

    /// Calculates the projection view matrix
    glm::mat4 projection_view() const;
};

}// namespace rt

#endif// REALTIME_CAMERA_H
