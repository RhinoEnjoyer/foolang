#include "lexer.hpp"

#include <string>
#include <map>

#include "lexer.tab.c"
namespace lexer{
  [[gnu::hot]]
  const int unwrap_token_type(int token_type, const source_range_t range, const char* const src){
    const std::string text = std::string(src + range.begin, range.end - range.begin + 1);
  
    switch(token_type){
    case BUILTIN:
      static const std::map<std::string, int> builtin_map = {
        {"@null", NULLPTR},
        {"@import", IMPORT},
        {"@true", TRUE}, {"@false", FALSE},
        {"@if", IF},{"@elif", ELIF},{"@else", ELSE},
        {"@for", FOR},
        {"@sizeof", SIZEOF},{"@typeof", TYPEOF},
        {"@fn", FN},{"@rec", REC},{"@union", UNION},{"@enum", ENUM},
        {"@alias", ALIAS},
        {"@ret", RET},{"@as",AS}
      };
  
      if(builtin_map.find(text) != builtin_map.end()) 
        token_type = builtin_map.at(text);
    break;
    case ID:
      static const std::map<std::string, int> builtin_type_map = {
        //int can have a arbitary length in the llvm ir so thing might not be the best way to go about it
        {"bool",S8},
        {"s8",S8},{"s32",S32},{"s16",S16},{"s64",S64},{"s128",S128},
        {"u8",U8},{"u32",U32},{"u16",U16},{"u64",U64},{"u128",U128},
        {"f16",F16},{"f32",F32},{"f64",F64},{"f128",F128},
      };
  
      if(builtin_type_map.find(text) != builtin_type_map.end())
        token_type = builtin_type_map.at(text);
    break;
    }
  
    return token_type;
  }
  
  
  
  const std::vector<token_t> lex(const char* const src, const uint64_t size){
      //Scanner
      yyscan_t scanner;
      YY_BUFFER_STATE buffer;
  
      yylex_init(&scanner);
      buffer = yy_scan_string(src,scanner);
  
  
      std::vector<token_t> tokens;
      int token_type;
      uint64_t buffer_loc = 0;
      do{
        token_type = yylex(scanner);
        if(token_type != NULLTKN) {
          const char* const buffer_text = yyget_text(scanner);
          const uint64_t text_len = strlen(buffer_text);
          const source_range_t range = {buffer_loc, buffer_loc + text_len - 1};
  
          if(token_type != WS) {
            const token_t token = {unwrap_token_type(token_type, range, src), 0, range, src};
            tokens.push_back(token); 
          }
  
          buffer_loc += text_len;
        }
  
      }while(token_type != NULLTKN);
  
      yy_delete_buffer(buffer,scanner);
      yylex_destroy(scanner);
  
      return tokens;
  }

};
