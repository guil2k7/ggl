// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#pragma once

#include "value.hh"

namespace gcl {

bool parse(Value& output, char const* chars, size_t length);

inline bool parse(Value& output, std::string_view text) {
    return parse(output, text.data(), text.length());
}

} // namespace gcl
