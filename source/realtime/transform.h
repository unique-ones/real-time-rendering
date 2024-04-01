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

#ifndef REALTIME_TRANSFORM_H
#define REALTIME_TRANSFORM_H

#include "realtime.h"

namespace rt::transform {

using Transform = glm::mat4;

/// Creates an orthographic transform
/// @param left The left plane
/// @param right The right plane
/// @param top The top plane
/// @param bottom The bottom plane
/// @param near The near plane
/// @param far The far plane
/// @return An orthogarphic transform
Transform orthographic(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far);

/// Creates a perspective transform
/// @param fov The vertical field of view
/// @param aspect The aspect ratio of the viewport
/// @param near The near plane
/// @param far The far plane
/// @return A perspective transform
Transform perspective(f32 fov, f32 aspect, f32 near, f32 far);

/// Creates a view direction transform
/// @param position The position of the eye
/// @param direction The direction in which the eye looks
/// @param up The up direction
/// @return A view direction transform
Transform view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = { 0.0f, -1.0f, 0.0f });

/// Creates a view target transform, which keeps the eye oriented at the target at all times
/// @param position The position of the eye
/// @param target The target to which the eye looks
/// @param up The up direction
/// @return A view target transform
Transform view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = { 0.0f, -1.0f, 0.0f });

/// Creates a view matrix using tait-bryan XYZ euler-angles
/// @param position The position of the eye
/// @param rotation The XYZ rotation
/// @return A view euler transform
Transform view_euler(glm::vec3 position, glm::vec3 rotation);

}// namespace rt::transform

#endif// REALTIME_TRANSFORM_H
