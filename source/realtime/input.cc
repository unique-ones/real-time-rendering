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

#include "input.h"
#include "window.h"

namespace rt {

namespace {

/// Checks if a vec3 is non zero
/// @param vec The vec3
/// @return A value that indicates whether the vec3 is non zero
bool vec3_non_zero(glm::vec3 vec) {
    return glm::dot(vec, vec) > std::numeric_limits<f32>::epsilon();
}

}// namespace

/// Moves an entity in the XZ plane
void Input::move_entity(Window &window, f32 dt, Entity &entity) const {
    auto rotation = glm::vec3{ 0.0f };
    auto native_handle = window.native_handle();
    if (glfwGetKey(native_handle, LOOK_RIGHT) == GLFW_PRESS) {
        rotation.y += 1.0f;
    }
    if (glfwGetKey(native_handle, LOOK_LEFT) == GLFW_PRESS) {
        rotation.y -= 1.0f;
    }
    if (glfwGetKey(native_handle, LOOK_UP) == GLFW_PRESS) {
        rotation.x += 1.0f;
    }
    if (glfwGetKey(native_handle, LOOK_DOWN) == GLFW_PRESS) {
        rotation.x -= 1.0f;
    }

    if (vec3_non_zero(rotation)) {
        entity.transform.rotation += look_speed * dt * glm::normalize(rotation);
    }

    constexpr auto half_pi = glm::half_pi<f32>();
    constexpr auto two_pi = glm::two_pi<f32>();
    entity.transform.rotation.x = glm::clamp(entity.transform.rotation.x, -half_pi, half_pi);
    entity.transform.rotation.y = glm::mod(entity.transform.rotation.y, two_pi);

    auto yaw = entity.transform.rotation.y;
    auto forward = glm::vec3{ glm::sin(yaw), 0.0f, glm::cos(yaw) };
    auto right = glm::vec3{ forward.y, 0.0f, -forward.x };
    auto up = glm::vec3{ 0.0f, -1.0f, 0.0f };

    auto move = glm::vec3{ 0.0f };
    if (glfwGetKey(native_handle, MOVE_FORWARD)) {
        move += forward;
    }
    if (glfwGetKey(native_handle, MOVE_BACKWARD)) {
        move -= forward;
    }
    if (glfwGetKey(native_handle, MOVE_RIGHT)) {
        move += right;
    }
    if (glfwGetKey(native_handle, MOVE_LEFT)) {
        move -= right;
    }
    if (glfwGetKey(native_handle, MOVE_UP)) {
        move += up;
    }
    if (glfwGetKey(native_handle, MOVE_DOWN)) {
        move -= up;
    }
    if (vec3_non_zero(move)) {
        entity.transform.translation += move_speed * dt * glm::normalize(move);
    }
}

}// namespace rt
