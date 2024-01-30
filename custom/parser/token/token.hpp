#pragma once
#include <string>

struct source_range_t{
  unsigned long begin;
  unsigned long end;
};

struct token_t{
  int type = 0;
  int subtype = 0;

  source_range_t loc = {0,0};
  const char* src = nullptr; 
};

static inline
const char* const token_text(const token_t* const t){
  return t->src + t->loc.begin;
}

static inline
const unsigned long token_len(const token_t* const t){
  return t->loc.end- t->loc.begin + 1;
}

static inline
const std::string token_as_str(const token_t* const t){
  return std::string(token_text(t),token_len(t));
}
