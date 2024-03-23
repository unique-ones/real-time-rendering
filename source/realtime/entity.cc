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

#include "entity.h"

namespace rt {

/// Creates a new transform component
Transform2DComponent::Transform2DComponent() : translation{}, scale{ 1.0f, 1.0f }, rotation{} { }

/// Retrieves the transform of the component
glm::mat2 Transform2DComponent::transform() const {
    auto s = glm::sin(rotation);
    auto c = glm::cos(rotation);
    glm::mat2 rot = { c, s, -s, c };
    glm::mat2 sc = { scale.x, 0.0f, 0.0f, scale.y };
    return rot * sc;
}

/// Creates a new entity
Entity Entity::create() {
    static identifier_type current = 0;
    return Entity{ current++ };
}

/// Retrieves the identifier of the entity
Entity::identifier_type Entity::identifier() const {
    return id;
}

/// Creates a new entity
Entity::Entity(identifier_type id) : id{ id } { }

}// namespace rt
