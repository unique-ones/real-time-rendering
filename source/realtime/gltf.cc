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

#include "gltf.h"
#include "json.h"

#include <string_view>

namespace rt {

namespace {

/// Consumes the given struct from a binary buffer
/// @param buffer The binary buffer
/// @return An optional struct if the struct fits into the buffer
template<typename T>
std::optional<T> consume(std::string_view &buffer) {
    if (sizeof(T) > buffer.size()) {
        return std::nullopt;
    }
    auto *obj = reinterpret_cast<const T *>(buffer.data());
    buffer.remove_prefix(sizeof(T));
    return *obj;
}

constexpr u32 GLB_HEADER_MAGIC = 0x46546C67;
constexpr u32 GLB_VERSION_SUPPORTED = 2;

}// namespace


/// Tries to read a GLB file from disk
std::optional<GlbFile> GlbFile::read(const fs::path &path) {
    auto content = read_file(path, std::ios::binary);
    if (not content) {
        return std::nullopt;
    }

    // The buffer where we seek around
    auto buffer = std::string_view{ *content };
    auto header = consume<Header>(buffer);
    if (not header or header->magic != GLB_HEADER_MAGIC or header->version > GLB_VERSION_SUPPORTED) {
        return std::nullopt;
    }

    auto chunk = consume<Chunk::Info>(buffer);
    if (not chunk or chunk->type != Chunk::Type::JSON) {
        return std::nullopt;
    }

    auto view = std::string_view{ buffer.data(), chunk->length };
    auto json = Json::parse(view);
    if (not json) {
        return std::nullopt;
    }

    return std::nullopt;
}

}// namespace rt