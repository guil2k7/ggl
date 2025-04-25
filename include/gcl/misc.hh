// Copyright 2025 Maicol Castro <maicolcastro.abc@gmail.com>.
// Distributed under the MIT License.
// See LICENSE.txt in the root directory of this project
// or at https://opensource.org/license/mit.

#pragma once

#include <cstddef>

namespace gcl {

struct Span {
    size_t beginLineNumber = 0;
    size_t beginColumnNumber = 0;

    size_t endLineNumber = 0;
    size_t endColumnNumber = 0;
};
        
} // namespace gcl
