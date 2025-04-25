// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#include <gcl/parser.hh>
#include "tokenizer.hh"

using namespace gcl;

class Parser {
public:
    bool parseValue(Value& output);
    void parseArray(Array& output);
    void parseDict(Dict& output);

    Tokenizer tokenizer;
};

bool gcl::parse(Value& output, char const* chars, size_t length) {
    Parser parser;
    parser.tokenizer.setText(chars, length);
    parser.tokenizer.advance();
    return parser.parseValue(output);
}

bool Parser::parseValue(Value& output) {
    Token& token = tokenizer.token();

    if (token.kind == TokenKind::Punctuaction) {
        if (token.data.punctuaction == Punctuaction::Lbrace) {
            output.type = ValueType::Dict;
            new (&output.data.dict) Dict();
            parseDict(output.data.dict);
        }
        else if (token.data.punctuaction == Punctuaction::Lsqb) {
            output.type = ValueType::Array;
            new (&output.data.array) Array();
            parseArray(output.data.array);
        }
    }
    else if (token.kind == TokenKind::String) {
        output.type = ValueType::String;
        new (&output.data.string) String(std::move(token.data.string));

        tokenizer.advance();
    }
    else if (token.kind == TokenKind::Int) {
        output.type = ValueType::Int;
        output.data.i = token.data.i;

        tokenizer.advance();
    }
    else if (token.kind == TokenKind::Float) {
        output.type = ValueType::Float;
        output.data.f = token.data.f;

        tokenizer.advance();
    }
    else if (token.kind == TokenKind::Identifier) {
        if (token.data.identifier == "true") {
            output.type = ValueType::Bool;
            output.data.b = true;
        }
        else if (token.data.identifier == "false") {
            output.type = ValueType::Bool;
            output.data.b = false;
        }
        else if (token.data.identifier == "null") {
            output.type = ValueType::Null;
            output.data.i = 0;
        }
        else {
            return false;
        }

        tokenizer.advance();
    }
    else {
        return false;
    }

    return true;
}

void Parser::parseArray(Array& output) {
    Token& token = tokenizer.token();

    // Eat the left square bracket.
    tokenizer.advance();

    for (;;) {
        Value value;

        if (!parseValue(value))
            throw GclException(GclErrorID::ExpectedValue, token.span, std::format("expected a value but found `{}`", token));

        output.push_back(std::move(value));

        if (token.kind != TokenKind::Punctuaction || token.data.punctuaction != Punctuaction::Comma) {
            if (token.kind == TokenKind::Punctuaction && token.data.punctuaction == Punctuaction::Rsqb)
                break;

            throw GclException(GclErrorID::ExpectedPunctuaction, token.span, std::format("expected `,` but found `{}`", token));
        }

        tokenizer.advance();
    }

    if (token.kind != TokenKind::Punctuaction || token.data.punctuaction != Punctuaction::Rsqb)
        throw GclException(GclErrorID::ExpectedPunctuaction, token.span, std::format("expected `]` but found `{}`", token));

    // Eat the right square bracket.
    tokenizer.advance();
}

void Parser::parseDict(Dict& output) {
    Token& token = tokenizer.token();

    // Eat the left brace.
    tokenizer.advance();

    for (;;) {
        if (token.kind != TokenKind::Identifier)
            break;

        std::string identifier = std::move(token.data.identifier);
        tokenizer.advance();

        if (token.kind != TokenKind::Punctuaction || token.data.punctuaction != Punctuaction::Colon)
            throw GclException(GclErrorID::ExpectedPunctuaction, token.span, std::format("expected `:` but found `{}`", token));

        tokenizer.advance();

        Value value;

        if (!parseValue(value))
            throw GclException(GclErrorID::ExpectedValue, token.span, std::format("expected a value but found `{}`", token));

        if (auto it = output.insert({ std::move(identifier), std::move(value) }); !it.second)
            throw GclException(GclErrorID::ExpectedPunctuaction, token.span, std::format("key `{}` already defined", it.first->first));

        if (token.kind != TokenKind::Punctuaction || token.data.punctuaction != Punctuaction::Comma) {
            if (token.kind == TokenKind::Punctuaction && token.data.punctuaction == Punctuaction::Rbrace)
                break;

            throw GclException(GclErrorID::ExpectedPunctuaction, token.span, std::format("expected `,` but found `{}`", token));
        }

        tokenizer.advance();
    }

    if (token.kind != TokenKind::Punctuaction || token.data.punctuaction != Punctuaction::Rbrace)
        throw GclException(GclErrorID::ExpectedPunctuaction, token.span, std::format("expected `}}` but found `{}`", token));

    // Eat the right brace.
    tokenizer.advance();
}
