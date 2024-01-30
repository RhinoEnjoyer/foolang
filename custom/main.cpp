#include "./parser/lexer/lexer.hpp"
#include "text_reader.hpp"
#include "./parser/parser.hpp"
#include <stdio.h>
#include <iostream>
#include <vector>


//change this to return the ast tree


int main(){
  const char* const unit = "../test.foo";
  uint8_t* b = nullptr;
  uint64_t size = 0;

  //readfile(unit,&b,&size);
  map_file(unit, &b, &size);
  
  if(!b){
    printf("Failed to read or allocate memory for the source buffer");
    exit(EXIT_FAILURE);
  }

  printf("Translation Unit:" YELLOW" %s" RESET "\n\n", unit);

  printf("Tokenizing...\n");
  std::vector<token_t> tokens = lexer::lex((char*)b, size);
  printf(GREEN"Done\n\n" RESET);

  //for(const auto& t: tokens){
  //   std::cout << token_code_print(t.type) << std::endl; 
  //}
  

  printf("Parsing...\n");
  parser::tree_t tree = parser::parse(tokens);
  printf(GREEN"Done\n\n" RESET);
  parser::node_traverse(&tree.head,parser::node_print); 


  umap_file(b, size);
  return 0;
}
