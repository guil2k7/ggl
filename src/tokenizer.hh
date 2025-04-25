// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <gcl/exception.hh>
#include <gcl/misc.hh>

namespace gcl {

enum class TokenKind {
    Eof,
    Int,
    Float,
    Identifier,
    String,
    Punctuaction,
};

enum class Punctuaction {
    Lbrace,
    Rbrace,
    Lsqb,
    Rsqb,
    Comma,
    Colon,
};

union TokenData {
    constexpr TokenData() : i{0} {}
    ~TokenData() {}

    uintptr_t i;
    float f;
    std::string identifier;
    std::string string;
    Punctuaction punctuaction;
};

class Token {
public:
    Span span;
    TokenKind kind;
    TokenData data;

    constexpr Token() : kind{TokenKind::Eof} {}

    inline Token(Token const& that) : span{that.span}, kind{that.kind} {
        that.copyDataTo(data);
    }

    inline Token(Token&& that) : span{that.span}, kind{that.kind} {
        that.moveDataTo(data);
        that.kind = TokenKind::Eof;
    }

    inline ~Token() {
        if (kind != TokenKind::Eof)
            releaseData();
    }

    inline Token& operator =(Token const& that) {
        if (kind != TokenKind::Eof)
            releaseData();

        that.copyDataTo(data);

        span = that.span;
        kind = that.kind;

        return *this;
    }

    inline Token& operator =(Token&& that) {
        if (kind != TokenKind::Eof)
            releaseData();

        that.moveDataTo(data);
        that.kind = TokenKind::Eof;

        span = that.span;
        kind = that.kind;

        return *this;
    }

    inline void reset() {
        if (kind != TokenKind::Eof) {
            releaseData();
            kind = TokenKind::Eof;
        }

        span = {};
    }

private:
    inline void copyDataTo(TokenData& dest) const {
        switch (kind) {
            case TokenKind::Int:
                dest.i = data.i;
                break;

            case TokenKind::Float:
                dest.f = data.i;
                break;

            case TokenKind::Identifier:
                new (&dest.identifier) std::string(data.identifier);
                break;

            case TokenKind::String:
                new (&dest.string) std::string(data.string);
                break;

            case TokenKind::Punctuaction:
                dest.punctuaction = data.punctuaction;
                break;

            default:
                break;
        }
    }

    inline void moveDataTo(TokenData& dest) {
        switch (kind) {
            case TokenKind::Int:
                dest.i = data.i;
                break;

            case TokenKind::Float:
                dest.f = data.i;
                break;

            case TokenKind::Identifier:
                new (&dest.identifier) std::string(std::move(data.identifier));
                break;

            case TokenKind::String:
                new (&dest.string) std::string(std::move(data.string));
                break;

            case TokenKind::Punctuaction:
                dest.punctuaction = data.punctuaction;
                break;

            default:
                break;
        }
    }

    inline void releaseData() {
        switch (kind) {
            case TokenKind::Identifier:
                data.identifier.~basic_string();
                break;

            case TokenKind::String:
                data.string.~basic_string();
                break;

            default:
                break;
        }
    }
};

class Tokenizer {
public:
    Tokenizer()
        : m_chars{}, m_length{0}
        , m_index{0}, m_lineNumber{1}, m_columNumber{0}
        , m_char{'\0'}, m_token()
    {}

    inline void setText(char const* chars, size_t length) {
        m_chars = chars;
        m_length = length;
        m_index = 0;
        m_lineNumber = 1;
        m_columNumber = 0;
        m_char = length > 0 ? m_chars[0] : '\0';
        m_token.reset();
    }

    inline void reset() {
        m_index = 0;
        m_lineNumber = 1;
        m_columNumber = 0;
        m_char = m_length > 0 ? m_chars[0] : '\0';
        m_token.reset();
    }

    inline Token& token() {
        return m_token;
    }

    inline Token const& token() const {
        return m_token;
    }

    bool advance();

private:
    char const* m_chars;
    size_t m_length;
    size_t m_index;
    size_t m_lineNumber;
    size_t m_columNumber;
    char m_char;
    Token m_token;

    bool advanceChar();
    void skipWhitespace();
    void skipComment();
    bool readIdentifier();
    bool readNumber();
    bool readPunctuaction();
    bool readString();
    bool readMisc();
};

} // namespace gcl

template<>
struct std::formatter<gcl::TokenKind> : std::formatter<char const*> {
    std::format_context::iterator format(gcl::TokenKind kind, std::format_context& context) const;
};

template<>
struct std::formatter<gcl::Punctuaction> : std::formatter<char const*> {
    std::format_context::iterator format(gcl::Punctuaction punctuaction, std::format_context& context) const;
};

template<>
struct std::formatter<gcl::Token> : std::formatter<char const*> {
    std::format_context::iterator format(gcl::Token const& token, std::format_context& context) const;
};
