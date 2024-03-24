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

#include "transform.h"

namespace rt::transform {

/// Creates an orthographic projection
Transform orthographic(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far) {
    Transform result{ 1.0f };
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (bottom - top);
    result[2][2] = 1.0f / (far - near);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(bottom + top) / (bottom - top);
    result[3][2] = -near / (far - near);
    return result;
}

/// Creates a perspective projection
Transform perspective(f32 fov, f32 aspect, f32 near, f32 far) {
    assert(glm::abs(aspect - std::numeric_limits<f32>::epsilon()) > 0.0f);
    auto tan_half_fov = glm::tan(fov / 2.0f);

    Transform result{ 1.0f };
    result[0][0] = 1.0f / (aspect * tan_half_fov);
    result[1][1] = 1.0f / (tan_half_fov);
    result[2][2] = far / (far - near);
    result[2][3] = 1.0f;
    result[3][2] = -(far * near) / (far - near);
    return result;
}

/// Creates a view direction transform
Transform view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    auto w = glm::normalize(direction);
    auto u = glm::normalize(glm::cross(w, up));
    auto v = glm::cross(w, u);

    Transform result{ 1.0f };
    result[0][0] = u.x;
    result[1][0] = u.y;
    result[2][0] = u.z;
    result[0][1] = v.x;
    result[1][1] = v.y;
    result[2][1] = v.z;
    result[0][2] = w.x;
    result[1][2] = w.y;
    result[2][2] = w.z;
    result[3][0] = -glm::dot(u, position);
    result[3][1] = -glm::dot(v, position);
    result[3][2] = -glm::dot(w, position);
    return result;
}

/// Creates a view target transform, which keeps the eye oriented at the target at all times
Transform view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    return view_direction(position, target - position, up);
}

/// Creates a view matrix using tait-bryan XYZ euler-angles
Transform view_euler(glm::vec3 position, glm::vec3 rotation) {
    auto c3 = glm::cos(rotation.z);
    auto s3 = glm::sin(rotation.z);
    auto c2 = glm::cos(rotation.x);
    auto s2 = glm::sin(rotation.x);
    auto c1 = glm::cos(rotation.y);
    auto s1 = glm::sin(rotation.y);

    glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };

    Transform result{ 1.0f };
    result[0][0] = u.x;
    result[1][0] = u.y;
    result[2][0] = u.z;
    result[0][1] = v.x;
    result[1][1] = v.y;
    result[2][1] = v.z;
    result[0][2] = w.x;
    result[1][2] = w.y;
    result[2][2] = w.z;
    result[3][0] = -glm::dot(u, position);
    result[3][1] = -glm::dot(v, position);
    result[3][2] = -glm::dot(w, position);
    return result;
}

}// namespace rt::transform
