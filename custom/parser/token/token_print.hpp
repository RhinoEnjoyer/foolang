#pragma once

#include <stdint.h>
#include <stdio.h>

#include "./token_codes.hpp"


#include "./token.hpp"
#include "./token_codes_print.hpp"

static
std::string token_print_type(const token_t* const token){ return token_code_print(token->type);}

#include "color_ansi_codes.h"
static inline void token_print(
  const token_t* const token,
  const char* const source
){
  printf(RED "\"%.*s\"" RESET ",", (int)token_len(token), token_text(token));
  printf(MAGENTA" %lu..%lu" RESET ",", token->loc.begin, token->loc.end);
  printf(YELLOW " %lu" RESET ",", token_len(token));
  printf(BLUE " "); std::cout << token_print_type(token) << std::endl; printf(RESET ",");
  printf("\n");
}

