#pragma once

#include <vector>
#include "../token/token.hpp"
#include "../token/token_codes.hpp"
#include <stdint.h>

namespace lexer {
  const std::vector<token_t> lex(const char* const src, const uint64_t size);
}

