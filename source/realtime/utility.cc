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

#include "utility.h"

#include <fstream>

namespace rt {

/// Tries to parse a unicode codepoint from a given string view
std::optional<u32> codepoint_from_view(std::string_view view) {
    u32 result{};
    if (std::from_chars(view.data(), view.data() + view.size(), result, 16).ec == std::errc{}) {
        return result;
    }
    return std::nullopt;
}

/// Print out an error message to the console and exit the application
/// with the specified error code
void error(s32 code, std::string_view message) {
    std::printf("[error] %.*s", static_cast<int>(message.size()), message.data());
    std::exit(code);
}

template<typename T>
std::partial_ordering order(const T &first, const T &second) {
    if (first == second) {
        return std::partial_ordering::equivalent;
    }
    return std::partial_ordering::unordered;
}

std::partial_ordering operator<=>(glm::vec1 first, glm::vec1 second) {
    return order(first, second);
}

std::partial_ordering operator<=>(const glm::vec2 &first, const glm::vec2 &second) {
    return order(first, second);
}

std::partial_ordering operator<=>(const glm::vec3 &first, const glm::vec3 &second) {
    return order(first, second);
}

std::partial_ordering operator<=>(const glm::vec4 &first, const glm::vec4 &second) {
    return order(first, second);
}

std::partial_ordering operator<=>(const glm::mat2 &first, const glm::mat2 &second) {
    return order(first, second);
}

std::partial_ordering operator<=>(const glm::mat3 &first, const glm::mat3 &second) {
    return order(first, second);
}

std::partial_ordering operator<=>(const glm::mat4 &first, const glm::mat4 &second) {
    return order(first, second);
}

}// namespace rt
