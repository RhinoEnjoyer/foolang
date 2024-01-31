#pragma once

#include "./token/token.hpp"
#include "./token/token_codes.hpp"
#include "./token/token_codes_print.hpp"
#include "../color_ansi_codes.h"
#include <vector>
#include <functional>

namespace parser{
  #define children_holder std::vector<node_t>

  struct node_t{
    size_t type = 0;
    children_holder children;
    token_t* token = nullptr;
  };

  void node_print(node_t* node,size_t depth);
  void node_traverse(node_t* head, std::function<void(node_t*,size_t)> action);

  struct tree_t{
    node_t head;
  };

  tree_t parse(std::vector<token_t>& tokens);
}
