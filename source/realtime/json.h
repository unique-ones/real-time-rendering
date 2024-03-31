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

#ifndef REALTIME_JSON_H
#define REALTIME_JSON_H

#include <unordered_map>
#include <variant>
#include <vector>

#include "utility.h"

namespace rt {

class Json {
public:
    // Trivial types
    using String = std::u32string;
    using Number = f64;
    using Bool = bool;
    using Null = std::nullptr_t;

    // Non-trivial type
    class Array;

    using Key = String;
    class Value;

    /// Creates an empty JSON object
    Json() = default;

    /// JSON object may be copied or moved
    Json(const Json &) = default;
    Json &operator=(const Json &) = default;
    Json(Json &&) = default;
    Json &operator=(Json &&) = default;

    /// Parses a JSON object from a string
    /// @param data The string
    /// @return An optional JSON object
    static std::optional<Json> parse(std::string_view data);

    /// Retrieves the value of the given key
    /// @param key The key
    /// @return A reference to the value
    Value &operator[](const Key &key);

    /// Retrieves the value of the given key
    /// @param key The key
    /// @return A reference to the value
    Value &operator[](std::string_view key);

    /// Retrieves the value of the given key
    /// @param key The key
    /// @return A reference to the value
    Value &get(const Key &key);

    /// Retrieves the value of the given key
    /// @param key The key
    /// @return A reference to the value
    Value &get(std::string_view key);

private:
    std::unordered_map<Key, Value> fields;
};

class Json::Array {
public:
    using Internal = std::vector<Value>;
    using Iterator = Internal::iterator;
    using ReverseIterator = Internal::reverse_iterator;
    using ConstIterator = Internal::const_iterator;
    using ConstReverseIterator = Internal::const_reverse_iterator;

    /// Creates an empty JSON array
    Array() = default;

    /// Retrieves the value at the specified index
    /// @param index The index
    /// @return A reference to the value
    Value &operator[](usize index);

    /// Adds a value to the array
    /// @param value The value
    void add(Value &&value);

    /// Adds a value to the array
    /// @param value The value
    void add(const Value &value);

    /// Retrieves the begin iterator of the array
    /// @return The begin iterator
    Iterator begin();

    /// Retrieves the end iterator of the array
    /// @return The end iterator
    Iterator end();

    /// Retrieves the reverse begin iterator of the array
    /// @return The reverse begin iterator
    ReverseIterator rbegin();

    /// Retrieves the reverse end iterator of the array
    /// @return The reverse end iterator
    ReverseIterator rend();

    /// Retrieves the const begin iterator of the array
    /// @return The const begin iterator
    ConstIterator cbegin() const;

    /// Retrieves the const end iterator of the array
    /// @return The const end iterator
    ConstIterator cend() const;

    /// Retrieves the const reverse begin iterator of the array
    /// @return The const reverse begin iterator
    ConstReverseIterator crbegin() const;

    /// Retrieves the const reverse end iterator of the array
    /// @return The const reverse end iterator
    ConstReverseIterator crend() const;

private:
    Internal fields;
};

class Json::Value {
public:
    /// Creates an empty JSON value
    Value() = default;

    /// A value may be copied or moved
    Value(const Value &) = default;
    Value &operator=(const Value &) = default;
    Value(Value &&) = default;
    Value &operator=(Value &&) = default;

    /// Creates a JSON string value
    /// @param string The string
    explicit Value(const String &string);

    /// Creates a JSON number value
    /// @param number The number
    explicit Value(Number number);

    /// Creates a JSON bool value
    /// @param boolean The bool
    explicit Value(Bool boolean);

    /// Creates a JSON nil value
    /// @param nil The nil
    explicit Value(Null nil);

    /// Creates a JSON object value
    /// @param json The object
    explicit Value(const Json &json);

    /// Creates a JSON array value
    /// @param array The array
    explicit Value(const Array &array);

    /// Tries to retrieve the value with the given type
    /// @tparam T The type
    /// @return An optional reference to the value
    template<typename T>
    T *as() {
        static_assert(is_same_as_any_v<T, String, Number, Bool, Null, Json, Array>,
                      "Invalid value type! Must be either String, Number, Bool, Null, Json or Array!");
        return std::get_if<T>(&value);
    }

private:
    std::variant<String, Number, Bool, Null, Json, Array> value;
};

namespace detail {

class JsonLexer {
public:
    /// List of valid tokens for parsing JSON
    enum class TokenType {
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        STRING,
        COLON,
        COMMA,
        TRUE,
        FALSE,
        NIL,
        NUMBER,
        END,
        INVALID
    };

    /// A JSON token with a type and its lexeme in the source string
    struct Token {
        TokenType type;
        std::string_view lexeme;
    };

    /// Creates a new lexer
    /// @param data The JSON data in string
    explicit JsonLexer(std::string_view data);

    /// Tokenizes the data
    /// @return A list of tokens
    std::vector<Token> tokenize();

    /// Tokenizes stringified JSON data
    /// @param data The stringified JSON data
    /// @return A list of tokens
    static std::vector<Token> tokenize(std::string_view data);

private:
    /// Tries to parse a number from the current data string
    /// @return An optional number
    std::optional<std::string_view> number();

    /// Tries to parse a string from the current data string
    /// @return An optional string
    std::optional<std::string_view> string();

    /// Advances the cursor by 'count' places
    /// @param count The number of places the cursor should be advanced
    void advance(u64 count = 1);

    /// Tries to consume a text, returns the lexeme if successful
    /// @param text The text
    /// @return The optional lexeme
    std::optional<std::string_view> consume(std::string_view text);

    /// Retrieves the current char or '\0' if the cursor reached the end
    /// @return The current char or '\0' if the cursor reached the end
    char current() const;

    /// Retrieves the next char or '\0' if the cursor reached the end
    /// @return The next char or '\0' if the cursor reached the end
    char next() const;

    /// Retrieves the data pointer or null if the cursor reached the end
    /// @return The data pointer or null if the cursor reached the end
    const char *current_ptr() const;

    /// Retrieves the current text given a count of places
    /// @param count The count of places to look forward
    /// @return The lexeme
    std::string_view current_text(u64 count = 1) const;

    /// Checks if the cursor reached the end
    /// @return A value that indicates whether the cursor reached the end
    bool end() const;

    std::string_view data;
    bool error;
};

class JsonParser {
public:
    /// Creates a new parser
    /// @param tokens The tokens
    explicit JsonParser(const std::vector<JsonLexer::Token> &tokens);

    /// Parses the JSON object
    /// @return An optional JSON object
    std::optional<Json> parse();

    /// Tries to parse stringified JSON data
    /// @param data The stringified JSON data
    /// @return An optional JSON object
    static std::optional<Json> parse(std::string_view data);

private:
    /// Tries to parse a JSON object
    /// @return An optional JSON object
    std::optional<Json::Value> object();

    /// Tries to parse a JSON value
    /// @return An optional JSON value
    std::optional<Json::Value> value();

    /// Tries to parse a JSON array value
    /// @return An optional JSON array value
    std::optional<Json::Value> array();

    /// Tries to parse a JSON string value
    /// @return An optional JSON string value
    std::optional<Json::Value> string();

    /// Tries to parse a JSON boolean value
    /// @return An optional JSON boolean value
    std::optional<Json::Value> boolean();

    /// Tries to parse a JSON nil value
    /// @return An optional JSON nil value
    std::optional<Json::Value> nil();

    /// Tries to parse a JSON number
    /// @return An optional JSON number
    std::optional<Json::Value> number();

    /// Advances the cursor by one
    void advance();

    /// Tries to consume a token
    /// @param token The token
    /// @return The optional token
    std::optional<JsonLexer::Token> consume(JsonLexer::TokenType token);

    /// Tries to match the current token type
    /// @param type The token type
    /// @return A value that indicates whether the current token type matches
    bool match(JsonLexer::TokenType type) const;

    /// Retrieves the current token or INVALID if the cursor reached the end
    /// @return The current token or INVALID if the cursor reached the end
    JsonLexer::Token current() const;

    /// Retrieves the next token or INVALID if the cursor reached the end
    /// @return The next token or INVALID if the cursor reached the end
    JsonLexer::Token next() const;

    /// Checks if the cursor reached the end
    /// @return A value that indicates whether the cursor reached the end
    bool end() const;

    std::vector<JsonLexer::Token> tokens;
};

}// namespace detail

}// namespace rt

#endif// REALTIME_JSON_H
