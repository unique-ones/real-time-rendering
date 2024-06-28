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
#include <chrono>

#include "application.h"
#include "buffer.h"
#include "glm/geometric.hpp"
#include "grid_system.h"
#include "input.h"
#include "realtime/frame_info.h"
#include "render_system.h"
#include "utility.h"

namespace rt {

namespace {

/// Computes the centroid of all entities
glm::vec3 compute_centroid(const std::vector<Entity> &entities) {
    auto result = glm::vec3{ 0.0f };
    for (const auto &entity : entities) {
        result += entity.mesh->centroid;
    }
    return result / static_cast<f32>(entities.size());
}

}// namespace

struct UniformBuffer {
    glm::mat4 projection_view{ 1.0f };
    glm::vec3 light_direction = glm::normalize(glm::vec3{ 1.0f, -3.0f, -1.0f });
};

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
    Buffer uniform_buffer{ device,
                           sizeof(UniformBuffer),
                           Swapchain::MAX_FRAMES_IN_FLIGHT,
                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                           device.physical_device_properties.limits.minUniformBufferOffsetAlignment };
    uniform_buffer.map();
    RenderSystem render_system{ device, renderer.swapchain_render_pass() };

    auto centroid = compute_centroid(entities);
    Camera camera{ window, centroid };

    auto last_time = std::chrono::high_resolution_clock::now();
    while (not window.should_close()) {
        glfwPollEvents();
        auto current_time = std::chrono::high_resolution_clock::now();
        auto frame_time = std::chrono::duration<f32>(current_time - last_time).count();
        last_time = current_time;

        camera.update(renderer.aspect_ratio());
        if (auto command_buffer = renderer.begin_frame()) {
            auto frame_index = renderer.frame_index();
            FrameInfo info{ frame_index, frame_time, command_buffer, camera };

            // Update
            UniformBuffer ubo{};
            ubo.projection_view = camera.projection_view();
            uniform_buffer.write(&ubo, static_cast<s32>(frame_index));
            uniform_buffer.flush(static_cast<s32>(frame_index));

            // Render
            renderer.begin_swapchain_render_pass(command_buffer, { 0.48f, 0.65f, 1.0f, 1.0f });
            render_system.render_entities(info, entities);
            renderer.end_swapchain_render_pass(command_buffer);
            renderer.end_frame();
        }
    }

    vkDeviceWaitIdle(device.logical_device);
}

/// Loads the meshes
void Application::load_entities() {
    auto &entity = entities.emplace_back(Entity::create());
    entity.mesh = Mesh::from_wavefront(device, "assets/stanford-dragon-10k.obj");
    entity.transform.scale = glm::vec3{ 1.0f };
    entity.transform.rotation = { glm::pi<f32>(), 0.0f, 0.0f };
}

}// namespace rt
