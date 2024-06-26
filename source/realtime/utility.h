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

#ifndef REALTIME_UTILITY_H
#define REALTIME_UTILITY_H

#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "realtime.h"
#include "realtime/utility.h"

namespace rt {

namespace fs = std::filesystem;

/// Checks whether the provided value is a nullptr. If so, the alternative
/// is returned.
/// @tparam Type The type of the value
/// @param value The value to check
/// @param alternative The alternative if the value is a nullptr
/// @return The value if it is not null, else the alternative
template<typename Type>
Type *value_or(Type *value, Type *alternative) {
    return value ? value : alternative;
}

/// Combines an arbitrary number of hash values together
/// @param seed The initial seed
/// @param v The first value
/// @param rest All other values
template<typename T, typename... Rest>
void hash_combine(usize &seed, const T &v, const Rest &...rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

/// Reads the file from the specified path
/// @param path The path to the file
/// @param flags Optional flags for opening the file (std::ios::binary, ..)
/// @return The content of the file, std::nullopt if the file does not
///         exist
template<typename CharType = char, typename StringType = std::basic_string<CharType>>
std::optional<StringType> read_file(const fs::path &path, std::ifstream::openmode flags = {}) {
    std::ifstream file(path, flags);
    if (not file.is_open()) {
        return std::nullopt;
    }

    return StringType{ std::istreambuf_iterator<CharType>(file), {} };
}

/// Tries to parse a number from a given string view
/// @tparam T The type of the number
/// @param view The view
/// @param base The number base
/// @return The optional number
template<typename T>
std::optional<T> number_from_view(std::string_view view) {
    T result{};
    if (std::from_chars(view.data(), view.data() + view.size(), result).ec == std::errc{}) {
        return result;
    }
    return std::nullopt;
}

/// Tries to parse a unicode codepoint from a given string view
/// @param view The string view
/// @return An optional unicode codepoint
std::optional<u32> codepoint_from_view(std::string_view view);

/// Print out an error message to the console and exit the application
/// with the specified error code
/// @param code The error code
/// @param message The message
[[noreturn]] void error(s32 code, std::string_view message);

namespace meta {

template<typename T, typename... Args>
struct is_same_as_any;

template<typename T, typename U>
struct is_same_as_any<T, U> : std::is_same<T, U> { };

template<typename T, typename First, typename... Rest>
struct is_same_as_any<T, First, Rest...>
    : std::conditional_t<std::is_same_v<T, First>, std::true_type, is_same_as_any<T, Rest...>> { };

/// Checks whether any type from a list of types is same as the first type
/// @tparam T The type to check against
/// @tparam Args The list of candidates
template<typename T, typename... Args>
inline constexpr bool is_same_as_any_v = is_same_as_any<T, Args...>::value;

}// namespace meta

std::partial_ordering operator<=>(glm::vec1 first, glm::vec1 second);
std::partial_ordering operator<=>(const glm::vec2 &first, const glm::vec2 &second);
std::partial_ordering operator<=>(const glm::vec3 &first, const glm::vec3 &second);
std::partial_ordering operator<=>(const glm::vec4 &first, const glm::vec4 &second);
std::partial_ordering operator<=>(const glm::mat2 &first, const glm::mat2 &second);
std::partial_ordering operator<=>(const glm::mat3 &first, const glm::mat3 &second);
std::partial_ordering operator<=>(const glm::mat4 &first, const glm::mat4 &second);


}// namespace rt

#endif// REALTIME_UTILITY_H
