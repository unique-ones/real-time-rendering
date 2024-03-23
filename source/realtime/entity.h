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

#ifndef REALTIME_ENTITY_H
#define REALTIME_ENTITY_H

#include <memory>

#include "model.h"

namespace rt {

struct Transform2DComponent {
    glm::vec2 translation;
    glm::vec2 scale;
    f32 rotation;

    /// Creates a new transform component
    Transform2DComponent();

    /// Retrieves the transform of the component
    /// @return The transform
    glm::mat2 transform() const;
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

    std::shared_ptr<Model> model;
    glm::vec3 color;
    Transform2DComponent transform;

private:
    /// Creates a new entity
    /// @param id The entity id
    explicit Entity(identifier_type id);

    identifier_type id;
};

}// namespace rt

#endif// REALTIME_ENTITY_H
