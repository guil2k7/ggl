// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#include <array>
#include "tokenizer.hh"

using namespace gcl;

std::format_context::iterator std::formatter<TokenKind>::format(TokenKind kind, std::format_context& context) const {
    switch (kind) {
        case TokenKind::Eof: return std::formatter<char const*>::format("eof", context);
        case TokenKind::Int: return std::formatter<char const*>::format("int", context);
        case TokenKind::Float: return std::formatter<char const*>::format("float", context);
        case TokenKind::Identifier: return std::formatter<char const*>::format("identifier", context);
        case TokenKind::String: return std::formatter<char const*>::format("string", context);
        case TokenKind::Punctuaction: return std::formatter<char const*>::format("punctuaction", context);
    }
}

std::format_context::iterator std::formatter<Punctuaction>::format(Punctuaction punctuaction, std::format_context& context) const {
    switch (punctuaction) {
        case Punctuaction::Lbrace: return std::formatter<char const*>::format("{", context);
        case Punctuaction::Rbrace: return std::formatter<char const*>::format("}", context);
        case Punctuaction::Lsqb: return std::formatter<char const*>::format("[", context);
        case Punctuaction::Rsqb: return std::formatter<char const*>::format("]", context);
        case Punctuaction::Comma: return std::formatter<char const*>::format(",", context);
        case Punctuaction::Colon: return std::formatter<char const*>::format(":", context);
    }
}

std::format_context::iterator std::formatter<Token>::format(Token const& token, std::format_context& context) const {
    switch (token.kind) {
        case TokenKind::Int:
            return std::formatter<intptr_t>{}.format(token.data.i, context);

        case TokenKind::Float:
            return std::formatter<intptr_t>{}.format(token.data.f, context);

        case TokenKind::Identifier:
            return std::formatter<std::string_view>{}.format(token.data.identifier, context);

        case TokenKind::String:
            return std::formatter<std::string_view>{}.format(token.data.string, context);

        case TokenKind::Punctuaction:
            return std::formatter<Punctuaction>{}.format(token.data.punctuaction, context);

        default:
            return std::formatter<TokenKind>{}.format(token.kind, context);
    }
}

static inline bool isAlpha(char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

static inline bool isDigit(char chr, int base = 10) {
    switch (base) {
        case 10:
            return chr >= '0' && chr <= '9';

        case 16:
            return (chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');

        case 2:
            return chr >= '0' && chr <= '1';
    }

    return false;
}

static inline bool isAlnum(char chr) {
    return isAlpha(chr) || isDigit(chr);
}

static inline int charToDigit(char chr, int base) {
    if (base <= 10)
        return chr - '0';

    if (base == 16) {
        if (chr <= '9')
            return chr - '0';

        if (chr <= 'F')
            return chr - 'A' + 10;

        if (chr <= 'f')
            return chr - 'a' + 10;
    }

    return 0;
}

bool Tokenizer::advance() {
    static std::array<bool (Tokenizer::*)(), 5> const READ_METHODS = {
        &Tokenizer::readIdentifier,
        &Tokenizer::readNumber,
        &Tokenizer::readPunctuaction,
        &Tokenizer::readString,
        &Tokenizer::readMisc,
    };

    skipWhitespace();

    while (m_char == '#') {
        skipComment();
        skipWhitespace();
    }

    m_token.reset();
    m_token.span.beginLineNumber = m_lineNumber;
    m_token.span.beginColumnNumber = m_columNumber;
    m_token.span.endLineNumber = m_lineNumber;
    m_token.span.endColumnNumber = m_columNumber;

    for (auto method : READ_METHODS) {
        if ((this->*method)())
            break;
    }

    m_token.span.endLineNumber = m_lineNumber;
    m_token.span.endColumnNumber = m_columNumber;

    return m_token.kind != TokenKind::Eof;
}

bool Tokenizer::advanceChar() {
    if (m_index + 1 >= m_length) {
        m_char = '\0';
        return false;
    }

    m_char = m_chars[++m_index];

    if (m_char == '\n') {
        m_lineNumber += 1;
        m_columNumber = 0;
    }
    else {
        m_columNumber += 1;
    }

    return true;
}

void Tokenizer::skipWhitespace() {
    for (;;) {
        switch (m_char) {
            case ' ':
            case '\t':
            case '\n':
                advanceChar();
                continue;

            default:
                break;
        }

        break;
    }
}

void Tokenizer::skipComment() {
    advanceChar();

    while (m_char != '\n') {
        if (!advanceChar())
            break;
    }
}

bool Tokenizer::readIdentifier() {
    if (!isAlpha(m_char))
        return false;

    std::string identifier;
    identifier.reserve(8);
    identifier.push_back(m_char);

    while (advanceChar() && (isAlnum(m_char) || m_char == '_'))
        identifier.push_back(m_char);

    m_token.kind = TokenKind::Identifier;
    new (&m_token.data) std::string(std::move(identifier));

    return true;
}

bool Tokenizer::readNumber() {
    // TODO: Add suport to floats.

    if (!isDigit(m_char) && m_char != '-' && m_char != '+')
        return false;

    bool isNeg = false;
    int base = 10;

    if (!isDigit(m_char)) {
        if (m_char == '-') {
            isNeg = true;
            advanceChar();
        }
        else if (m_char == '+') {
            // isNeg = false;
            advanceChar();
        }
        else {
            throw GclException(GclErrorID::UnknownChar, m_token.span, std::format("unknown character `{}`", m_char));
        }
    }

    if (m_char == '0') {
        advanceChar();

        if (!isDigit(m_char)) {
            switch (m_char) {
                case 'b':
                case 'B':
                    base = 2;
                    break;

                case 'x':
                case 'X':
                    base = 16;
                    break;

                default:
                    m_token.kind = TokenKind::Int;
                    m_token.data.i = 0;
                    return true;
            }

            advanceChar();

            if (!isDigit(m_char, base))
                throw GclException(GclErrorID::InvalidDigit, m_token.span, std::format("invalid digit `{}` for base {}", m_char, base));
        }
    }

    uintptr_t value = charToDigit(m_char, base);

    while (advanceChar() && isDigit(m_char, base)) {
        value *= base;
        value += charToDigit(m_char, base);
    }

    if (isAlnum(m_char)) {
        char invalidDigitChr = m_char;

        advanceChar();

        while (isAlnum(m_char))
            advanceChar();

        throw GclException(GclErrorID::InvalidDigit, m_token.span, std::format("invalid digit `{}` for base {}", invalidDigitChr, base));
    }

    if (isNeg)
        value = -value;

    m_token.kind = TokenKind::Int;
    m_token.data.i = value;

    return true;
}

bool Tokenizer::readPunctuaction() {
    Punctuaction punctuaction;

    switch (m_char) {
        case '{': punctuaction = Punctuaction::Lbrace; break;
        case '}': punctuaction = Punctuaction::Rbrace; break;
        case '[': punctuaction = Punctuaction::Lsqb; break;
        case ']': punctuaction = Punctuaction::Rsqb; break;
        case ',': punctuaction = Punctuaction::Comma; break;
        case ':': punctuaction = Punctuaction::Colon; break;

        default:
            return false;
    }

    advanceChar();

    m_token.kind = TokenKind::Punctuaction;
    m_token.data.punctuaction = punctuaction;

    return true;
}

bool Tokenizer::readString() {
    if (m_char != '"')
        return false;

    std::string string;
    string.reserve(10);

    while (advanceChar() && m_char != '"') {
        if (m_char == '\n')
            throw GclException(GclErrorID::ExpectedStringEnd, m_token.span, std::format("expected string end"));

        if (m_char == '\\') {
            advanceChar();

            switch (m_char) {
                case 'n':
                    string.push_back('\n');
                    break;

                case 't':
                    string.push_back('\t');
                    break;

                case '\\':
                    string.push_back('\\');
                    break;

                case '"':
                    string.push_back('"');
                    break;

                default:
                    throw GclException(GclErrorID::InvalidEscape, m_token.span, std::format("invalid escape sequence `{}`", m_char));
            }
        }
        else {
            string.push_back(m_char);
        }
    }

    if (m_char != '"')
        throw GclException(GclErrorID::ExpectedStringEnd, m_token.span, std::format("expected string end"));

    advanceChar();

    m_token.kind = TokenKind::String;
    new (&m_token.data.string) std::string(std::move(string));

    return true;
}

bool Tokenizer::readMisc() {
    switch (m_char) {
        case '\0':
            m_token.kind = TokenKind::Eof;
            break;

        default:
            throw GclException(GclErrorID::UnknownChar, m_token.span, std::format("unknown character `{}`", m_char));
    }

    return true;
}
