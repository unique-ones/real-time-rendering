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

#ifndef REALTIME_ENTITY_H
#define REALTIME_ENTITY_H

#include <memory>

#include "mesh.h"

namespace rt {

struct TransformComponent {
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 rotation;

    /// Creates a new transform component
    TransformComponent();

    /// Retrieves the transform of the component, which corresponds to translation * Ry * Rx * Ry * scale
    /// Note that the rotation convention uses tait-bryan angles with axis order Y(1), X(2) and Z(3)
    /// @return The transform
    glm::mat4 transform() const;

    /// Calculates the normal matrix
    /// @return The normal matrix
    glm::mat4 normal() const;
};

class Entity {
public:
    using identifier_type = u32;

    /// An entity cannot be copied, allow move
    Entity(const Entity &) = delete;
    Entity &operator=(const Entity &) = delete;
    Entity(Entity &&) = default;
    Entity &operator=(Entity &&) = default;

    /// Creates a new entity
    /// @return The new entity
    static Entity create();

    /// Retrieves the identifier of the entity
    /// @return The identifier
    identifier_type identifier() const;

    std::shared_ptr<Mesh> mesh;
    glm::vec3 color{};
    TransformComponent transform;

private:
    /// Creates a new entity
    /// @param id The entity id
    explicit Entity(identifier_type id);

    identifier_type id;
};

}// namespace rt

#endif// REALTIME_ENTITY_H
