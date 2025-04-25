// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#pragma once

#include <exception>
#include <string>
#include "misc.hh"

namespace gcl {

enum class GclErrorID {
    ExpectedIdentifier,
    ExpectedNumber,
    ExpectedPunctuaction,
    ExpectedStringEnd,
    ExpectedValue,
    KeyAlreadyDefined,
    InvalidDigit,
    InvalidEscape,
    UnknownChar,
};

class GclException : public std::exception {
public:
    GclException(GclErrorID errorID, Span span, std::string&& info) :
        span{span}, info{std::move(info)} {}

    inline char const* what() const noexcept {
        return info.c_str();
    }

    GclErrorID errorID;
    Span span;
    std::string info;
};

} // namespace gcl
