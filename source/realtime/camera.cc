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

#include "camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace rt {

Camera::Camera(Window &window, const glm::vec3 &target)
    : window{ window },
      azimuth{ 90.0f },
      declination{},
      distance{ 50.0f },
      target{ target },
      projection{ 1.0f },
      view{ 1.0f },
      clicked{},
      cursor_start{},
      cursor_current{} {
    window.register_listener({ .type = EventType::Scroll, .handler = [this](const Event &e) {
                                  auto scroll_event = dynamic_cast<const ScrollEvent &>(e);
                                  distance -= static_cast<f32>(scroll_event.y);
                              } });
    window.register_listener({ .type = EventType::Cursor, .handler = [this](const Event &e) {
                                  auto cursor_event = dynamic_cast<const CursorEvent &>(e);
                                  if (clicked) {
                                      cursor_current = glm::vec2{ cursor_event.x, cursor_event.y };
                                  } else {
                                      cursor_start = glm::vec2{ cursor_event.x, cursor_event.y };
                                      cursor_current = cursor_start;
                                  }
                              } });
}

void Camera::update(f32 aspect) {
    auto offset = cursor_current - cursor_start;
    azimuth += offset.x;
    declination -= offset.y;

    // clamp values
    azimuth = glm::mod(azimuth, 360.0f);
    declination = glm::clamp(declination, -89.0f, 89.0f);

    auto az = glm::radians(azimuth);
    auto dec = glm::radians(declination);

    glm::vec3 position{};
    position.x = target.x + distance * glm::cos(dec) * glm::cos(az);
    position.y = target.y + distance * glm::sin(dec);
    position.z = target.z + distance * glm::cos(dec) * glm::sin(az);

    auto front = glm::normalize(target - position);
    auto right = glm::normalize(glm::cross(front, { 0.0f, 1.0f, 0.0f }));
    auto up = glm::normalize(glm::cross(right, front));
    view = glm::lookAt(position, target, up);
    projection = glm::perspective(45.0f, aspect, 0.01f, 100.0f);

    // Update state
    cursor_start = cursor_current;
    clicked = glfwGetMouseButton(window.native_handle(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}

/// Calculates the projection view matrix
glm::mat4 Camera::projection_view() const {
    return projection * view;
}

}// namespace rt
