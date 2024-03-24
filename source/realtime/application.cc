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

#include <array>

#include "application.h"
#include "render_system.h"
#include "utility.h"

namespace rt {

namespace {

/// Creates a cube model
/// @param device The Vulkan d1evice
/// @param offset The offset of the model
/// @return A cube model
std::unique_ptr<Model> create_cube_model(Device &device, glm::vec3 offset) {
    std::vector<Model::Vertex> vertices{
        // left face (white)
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },

        // right face (yellow)
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },

        // top face (orange, remember y axis points down)
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },

        // bottom face (red)
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },

        // nose face (blue)
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },

        // tail face (green)
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
    };

    for (auto &[position, _] : vertices) {
        position += offset;
    }

    return std::make_unique<Model>(device, vertices);
}

}// namespace

/// Creates a realtime application
Application::Application(Specification specification)
    : window{ std::move(specification) },
      device{ window },
      renderer{ window, device } {
    load_entities();
}

/// Destroys all application objects
Application::~Application() = default;

/// Runs the application
void Application::run() {
    RenderSystem render_system{ device, renderer.swapchain_render_pass() };
    Camera camera{};
    camera.view = transform::view_target(glm::vec3{ -1.0f, -2.0f, 2.0f }, glm::vec3{ 0.0f, 0.0f, 2.5f });

    auto last_frame = static_cast<f32>(glfwGetTime());

    while (not window.should_close()) {
        glfwPollEvents();

        auto aspect = renderer.aspect_ratio();
        // camera.projection = transform::orthographic(-aspect, aspect, -1, 1, -1, 1);
        camera.projection = transform::perspective(glm::radians(50.0f), aspect, 0.1f, 10.0f);

        if (auto command_buffer = renderer.begin_frame()) {
            auto now = static_cast<f32>(glfwGetTime());
            auto dt = now - last_frame;
            last_frame = now;

            renderer.begin_swapchain_render_pass(command_buffer);
            render_system.render_entities(command_buffer, entities, camera, dt);
            renderer.end_swapchain_render_pass(command_buffer);
            renderer.end_frame();
        }
    }

    vkDeviceWaitIdle(device.logical_device);
}

/// Loads the models
void Application::load_entities() {
    auto &cube = entities.emplace_back(Entity::create());
    cube.model = std::move(create_cube_model(device, glm::vec3{ 0.0f }));
    cube.transform.translation = { 0.0f, 0.0f, 2.5f };
    cube.transform.scale = { 0.5f, 0.5f, 0.5f };
}

}// namespace rt
