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

#include <charconv>
#include <unordered_set>

#include "json.h"

namespace rt {

/// Parses a JSON object from a string
std::optional<Json> Json::parse(std::string_view data) {
    return detail::JsonParser::parse(data);
}

/// Retrieves the value of the given key
Json::Value &Json::operator[](const Key &key) {
    return get(key);
}

/// Retrieves the value of the given key
Json::Value &Json::operator[](std::string_view key) {
    return get(key);
}

/// Retrieves the value of the given key
Json::Value &Json::get(const Key &key) {
    return fields[key];
}

/// Retrieves the value of the given key
Json::Value &Json::get(std::string_view key) {
    Key k{ key.begin(), key.end() };
    return get(k);
}

/// Creates a JSON string value
Json::Value::Value(const String &string) : value{ string } { }

/// Creates a JSON number value
Json::Value::Value(Number number) : value{ number } { }

/// Creates a JSON bool value
Json::Value::Value(Bool boolean) : value{ boolean } { }

/// Creates a JSON nil value
Json::Value::Value(Null nil) : value{ nil } { }

/// Creates a JSON object value
Json::Value::Value(const Json &json) : value{ json } { }

/// Creates a JSON array value
Json::Value::Value(const Array &array) : value{ array } { }

/// Retrieves the value at the specified index
Json::Value &Json::Array::operator[](usize index) {
    return fields[index];
}

/// Adds a value to the array
void Json::Array::add(Value &&value) {
    fields.emplace_back(std::move(value));
}

/// Adds a value to the array
void Json::Array::add(const Value &value) {
    fields.emplace_back(value);
}

/// Retrieves the begin iterator of the array
Json::Array::Iterator Json::Array::begin() {
    return fields.begin();
}

/// Retrieves the end iterator of the array
Json::Array::Iterator Json::Array::end() {
    return fields.end();
}

/// Retrieves the reverse begin iterator of the array
Json::Array::ReverseIterator Json::Array::rbegin() {
    return fields.rbegin();
}

/// Retrieves the reverse end iterator of the array
Json::Array::ReverseIterator Json::Array::rend() {
    return fields.rend();
}

/// Retrieves the const begin iterator of the array
Json::Array::ConstIterator Json::Array::cbegin() const {
    return fields.cbegin();
}

/// Retrieves the const end iterator of the array
Json::Array::ConstIterator Json::Array::cend() const {
    return fields.cend();
}

/// Retrieves the const reverse begin iterator of the array
Json::Array::ConstReverseIterator Json::Array::crbegin() const {
    return fields.crbegin();
}

/// Retrieves the const reverse end iterator of the array
Json::Array::ConstReverseIterator Json::Array::crend() const {
    return fields.crend();
}

namespace detail {

namespace {

/// Checks whether the provided character is a trivial control character
/// @param c The provided character
/// @return A value that indicates whether the provided character is a trivial control character
bool trivial_control_character(char c) {
    static const std::unordered_set known = {
        '"', '\\', '/', 'b', 'f', 'n', 'r', 't',
    };
    return known.contains(c);
}

/// Checks whether the given text consists only of hex digits
/// @param text The given text
/// @return A value that indicates whether the given text consists only of hex digits
bool is_hex(std::string_view text) {
    return std::ranges::all_of(
            text, [](char c) { return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F'); });
}

/// Stringifies a string view by replacing control characters with actual ones
/// @param view The string view
/// @return An optional stringified string
std::optional<Json::String> stringify(std::string_view view) {
    Json::String result;
    while (!view.empty()) {
        if (view.front() == '\\') {
            view.remove_prefix(1);
            switch (view.front()) {
                case '"':
                    result += '"';
                    view.remove_prefix(1);
                    break;
                case '\\':
                    result += '\\';
                    view.remove_prefix(1);
                    break;
                case 'b':
                    result += '\b';
                    view.remove_prefix(1);
                    break;
                case 'f':
                    result += '\f';
                    view.remove_prefix(1);
                    break;
                case 'n':
                    result += '\n';
                    view.remove_prefix(1);
                    break;
                case 'r':
                    result += '\r';
                    view.remove_prefix(1);
                    break;
                case 't':
                    result += '\t';
                    view.remove_prefix(1);
                    break;
                case 'u': {
                    view.remove_prefix(1);
                    if (auto number = codepoint_from_view(view.substr(0, 4))) {
                        result += static_cast<Json::String::value_type>(*number);
                        view.remove_prefix(4);
                    } else {
                        return std::nullopt;
                    }
                    break;
                }
                default:
                    return std::nullopt;
            }
        } else {
            result += view.front();
            view.remove_prefix(1);
        }
    }
    return result;
}

constexpr auto TOKEN_INVALID = JsonLexer::Token{ JsonLexer::TokenType::INVALID, "" };

}// namespace

/// Creates a new lexer
JsonLexer::JsonLexer(std::string_view data) : data{ data }, error{ false } { }

/// Tokenizes the data
std::vector<JsonLexer::Token> JsonLexer::tokenize() {
    std::vector<Token> result{};

    while (not end() and not error) {
        switch (current()) {
            case '{':
                result.emplace_back(TokenType::LEFT_BRACE, current_text());
                advance();
                continue;
            case '}':
                result.emplace_back(TokenType::RIGHT_BRACE, current_text());
                advance();
                continue;
            case '[':
                result.emplace_back(TokenType::LEFT_BRACKET, current_text());
                advance();
                continue;
            case ']':
                result.emplace_back(TokenType::RIGHT_BRACKET, current_text());
                advance();
                continue;
            case ':':
                result.emplace_back(TokenType::COLON, current_text());
                advance();
                continue;
            case ',':
                result.emplace_back(TokenType::COMMA, current_text());
                advance();
                continue;
            default:
                break;
        }

        if (std::isspace(current())) {
            advance();
        } else if (auto tr = consume("true")) {
            result.emplace_back(TokenType::TRUE, *tr);
        } else if (auto fal = consume("false")) {
            result.emplace_back(TokenType::FALSE, *fal);
        } else if (auto nil = consume("null")) {
            result.emplace_back(TokenType::NIL, *nil);
        } else if (auto num = number()) {
            result.emplace_back(TokenType::NUMBER, *num);
        } else if (auto str = string()) {
            result.emplace_back(TokenType::STRING, *str);
        } else {
            error = true;
        }
    }

    if (not error) {
        result.emplace_back(TokenType::END, "");
        return result;
    }
    return {};
}

/// Tokenizes stringified JSON data
std::vector<JsonLexer::Token> JsonLexer::tokenize(std::string_view data) {
    auto lex = JsonLexer{ data };
    return lex.tokenize();
}

/// Tries to parse a number from the current data string
std::optional<std::string_view> JsonLexer::number() {
    auto c = current();
    auto sign = c == '-';
    if (not std::isdigit(c) and not sign) {
        return std::nullopt;
    }

    auto *begin = current_ptr();
    if (sign) {
        advance();
    }

    while (std::isdigit(current())) {
        advance();
    }

    if (current() == '.') {
        if (not std::isdigit(next())) {
            error = true;
            return std::nullopt;
        }
        advance(2);
        while (std::isdigit(current())) {
            advance();
        }
    }
    if (auto exp = current(); exp == 'e' or exp == 'E') {
        advance();
        if (auto sgn = current(); sgn == '+' or sgn == '-' or std::isdigit(sgn)) {
            advance();
            while (std::isdigit(current())) {
                advance();
            }
        } else {
            // malformed
            error = true;
            return std::nullopt;
        }
    }
    return std::string_view{ begin, current_ptr() };
}

/// Tries to parse a string from the current data string
std::optional<std::string_view> JsonLexer::string() {
    if (current() != '"') {
        return std::nullopt;
    }
    advance();
    auto *begin = current_ptr();
    while (current() != '"') {
        if (current() == '\\') {
            advance();
            if (auto c = current(); trivial_control_character(c)) {
                advance();
            } else if (c == 'u') {
                advance();
                if (not is_hex(current_text(4))) {
                    error = true;
                    return std::nullopt;
                }
                advance(4);
            } else {
                error = true;
                return std::nullopt;
            }
        } else {
            advance();
        }
    }
    auto *end = current_ptr();
    advance();
    return std::string_view{ begin, end };
}

/// Advances the cursor by 'count' places
void JsonLexer::advance(u64 count) {
    data.remove_prefix(count);
}

/// Tries to consume a text, returns the lexeme if successful
std::optional<std::string_view> JsonLexer::consume(std::string_view text) {
    auto len = text.length();
    if (data.length() < len) {
        return {};
    }
    if (auto sub = data.substr(0, len); sub == text) {
        advance(len);
        return sub;
    }
    return {};
}

/// Retrieves the current char or '\0' if the cursor reached the end
char JsonLexer::current() const {
    return not end() ? data.front() : '\0';
}

/// Retrieves the next char or '\0' if the cursor reached the end
char JsonLexer::next() const {
    return data.size() > 1 ? data[1] : '\0';
}

/// Retrieves the data pointer or null if the cursor reached the end
const char *JsonLexer::current_ptr() const {
    return not end() ? data.data() : nullptr;
}

/// Retrieves the current text given a count of places
std::string_view JsonLexer::current_text(u64 count) const {
    return not end() ? data.substr(0, count) : std::string_view{};
}

/// Checks if the cursor reached the end
bool JsonLexer::end() const {
    return data.empty();
}

/// Creates a new parser
JsonParser::JsonParser(const std::vector<JsonLexer::Token> &tokens) : tokens{ tokens } { }

/// Parses the JSON object
std::optional<Json> JsonParser::parse() {
    if (auto obj = object()) {
        if (auto json = obj->as<Json>()) {
            return *json;
        }
    }
    return std::nullopt;
}

/// Tries to parse stringified JSON data
std::optional<Json> JsonParser::parse(std::string_view data) {
    auto tokens = JsonLexer::tokenize(data);
    auto parser = JsonParser{ tokens };
    return parser.parse();
}

/// Tries to parse a JSON object
std::optional<Json::Value> JsonParser::object() {
    if (not consume(JsonLexer::TokenType::LEFT_BRACE)) {
        return std::nullopt;
    }

    Json json;
    while (not consume(JsonLexer::TokenType::RIGHT_BRACE)) {
        if (auto key_token = consume(JsonLexer::TokenType::STRING)) {
            auto key = stringify(key_token->lexeme);
            if (not consume(JsonLexer::TokenType::COLON)) {
                return std::nullopt;
            }

            auto val = value();
            if (not key or not val) {
                return std::nullopt;
            }

            json[*key] = std::move(*val);

            if (not match(JsonLexer::TokenType::RIGHT_BRACE) and not consume(JsonLexer::TokenType::COMMA)) {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }

    return Json::Value{ json };
}

/// Tries to parse a JSON value
std::optional<Json::Value> JsonParser::value() {
    auto token = current();
    switch (token.type) {
        case JsonLexer::TokenType::LEFT_BRACE:
            return object();
        case JsonLexer::TokenType::LEFT_BRACKET:
            return array();
        case JsonLexer::TokenType::STRING:
            return string();
        case JsonLexer::TokenType::TRUE:
        case JsonLexer::TokenType::FALSE:
            return boolean();
        case JsonLexer::TokenType::NIL:
            return nil();
        case JsonLexer::TokenType::NUMBER:
            return number();
        default:
            break;
    }
    return std::nullopt;
}

/// Tries to parse a JSON array
std::optional<Json::Value> JsonParser::array() {
    if (not consume(JsonLexer::TokenType::LEFT_BRACKET)) {
        return std::nullopt;
    }

    Json::Array array;
    while (not consume(JsonLexer::TokenType::RIGHT_BRACKET)) {
        auto val = value();
        if (not val) {
            return std::nullopt;
        }
        array.add(*val);

        if (not match(JsonLexer::TokenType::RIGHT_BRACKET) and not consume(JsonLexer::TokenType::COMMA)) {
            return std::nullopt;
        }
    }

    return Json::Value{ array };
}

/// Tries to parse a JSON string value
std::optional<Json::Value> JsonParser::string() {
    if (auto str = consume(JsonLexer::TokenType::STRING)) {
        if (auto parsed = stringify(str->lexeme)) {
            return Json::Value{ *parsed };
        }
    }
    return std::nullopt;
}

/// Tries to parse a JSON boolean value
std::optional<Json::Value> JsonParser::boolean() {
    if (consume(JsonLexer::TokenType::TRUE)) {
        return Json::Value{ true };
    }
    if (consume(JsonLexer::TokenType::FALSE)) {
        return Json::Value{ false };
    }
    return std::nullopt;
}

/// Tries to parse a JSON nil value
std::optional<Json::Value> JsonParser::nil() {
    if (consume(JsonLexer::TokenType::NIL)) {
        return Json::Value{ nullptr };
    }
    return std::nullopt;
}

/// Tries to parse a JSON number
std::optional<Json::Value> JsonParser::number() {
    if (auto num = consume(JsonLexer::TokenType::NUMBER)) {
        if (auto parsed = number_from_view<Json::Number>(num->lexeme)) {
            return Json::Value{ *parsed };
        }
    }
    return std::nullopt;
}

/// Advances the cursor by 1
void JsonParser::advance() {
    tokens = { tokens.begin() + 1, tokens.end() };
}

/// Tries to consume a token
std::optional<JsonLexer::Token> JsonParser::consume(JsonLexer::TokenType token) {
    if (match(token)) {
        auto consumed = current();
        advance();
        return consumed;
    }
    return {};
}

/// Tries to match the current token type
bool JsonParser::match(JsonLexer::TokenType type) const {
    return current().type == type;
}

/// Retrieves the current token or INVALID if the cursor reached the end
JsonLexer::Token JsonParser::current() const {
    return end() ? TOKEN_INVALID : tokens.front();
}

/// Retrieves the next token or INVALID if the cursor reached the end
JsonLexer::Token JsonParser::next() const {
    return tokens.size() > 1 ? tokens[1] : TOKEN_INVALID;
}

/// Checks if the cursor reached the end
bool JsonParser::end() const {
    if (tokens.empty() or tokens.front().type == JsonLexer::TokenType::END) {
        return true;
    }
    return false;
}

}// namespace detail

}// namespace rt
