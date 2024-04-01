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

#ifndef REALTIME_INPUT_H
#define REALTIME_INPUT_H

#include "entity.h"
#include "window.h"

namespace rt {

struct Input {
    constexpr static auto MOVE_SPEED_DEFAULT = 3.0f;
    constexpr static auto LOOK_SPEED_DEFAULT = 1.5f;

    enum KeyMapping {
        MOVE_LEFT = GLFW_KEY_A,
        MOVE_RIGHT = GLFW_KEY_D,
        MOVE_FORWARD = GLFW_KEY_W,
        MOVE_BACKWARD = GLFW_KEY_S,
        MOVE_UP = GLFW_KEY_E,
        MOVE_DOWN = GLFW_KEY_Q,
        LOOK_LEFT = GLFW_KEY_LEFT,
        LOOK_RIGHT = GLFW_KEY_RIGHT,
        LOOK_UP = GLFW_KEY_UP,
        LOOK_DOWN = GLFW_KEY_DOWN
    };

    f32 move_speed = MOVE_SPEED_DEFAULT;
    f32 look_speed = LOOK_SPEED_DEFAULT;

    /// Moves an entity in the XZ plane
    /// @param window The window handle
    /// @param dt The time step
    /// @param entity The entity
    void move_entity(Window &window, f32 dt, Entity &entity) const;
};

}// namespace rt

#endif// REALTIME_INPUT_H
