#!/bin/python 
import re

TOKENS="""
  NULLTKN WS HEX BIN ID
  STRLIT
  INT TONE_INT FLOAT
  ASIGN
  PLUS MINUS MULT DIV MOD
  POINTER DEREF
  AND OR XOR NOT
  SHIFTL SHIFTR
  EQUAL NEQUAL GREATER GEQUAL LESS LEQUAL
  LPAREN RPAREN
  LBRACE RBRACE
  LCBRACE RCBRACE
  TONE DOT COLON SEMICOLON COMMA
  BUILTIN
  TEMPLATE
  COMMENT OTHER
  VALUE
  IMPORT
  TRUE FALSE
  FOR
  IF ELSE ELIF
  SIZEOF TYPEOF OFFSETOF
  FN REC UNION ENUM
  STMT
  STMT_LIST
  IF_STMT
  ELIF_STMT
  ELSE_STMT
  IF_CHAIN_STMT
  IF_COND
  VAR_DECL_STMT
  RET
  ALIAS
  NULLPTR

  ERROR

  UNIT EXT_DECL
  TYPE
  BASE_TYPE
  USER_TYPE
  BOOL S8 S16 S32 S64 S128
  U8 U16 U32 U64 U128
  F16 F32 F64 F128.
  CBRACES
  PARENS
  EMPTY
  ID_DECL
  ID_LIST

  UN_OP
  UN_OP_EXPR
  NASSOC_OP
  RIGHT_OP
  LEFT_OP

  EMPTY_LIST
  INIT_LIST
  EMPTY_GROUP
  SEMI_LIST
  COMMA_LIST
  
  DECL_PREFIX
  DECL
  LIST
  LITERAL
  EXT_DECL_TYPE

  ASIGN_STMT
  
  FN_DECL
  FN_PROTO
  FN_CALL

  NAMESPACE
  REC_DECL
  ENUM_DECL
  UNION_DECL
  FN_TYPE
  REC_TYPE
  ENUM_TYPE
  UNION_TYPE


  FN_ARGS_RETS
  ARGS
  RETS
    
  LVALUE
  RVALUE
  VAR_DECL
  CAST
  AS
  MEMBER
 
  WRAP
  GROUP
  EXPR
  EXPR_STMT
  COMP_STMT
  TEST
"""

pattern = r"([A-Z_0-9]+)"

matches = re.finditer(pattern, TOKENS, re.MULTILINE)

tokens = []
define_tokens = "#pragma once\n"
for i, m in enumerate(matches,start=1):
  token = m.group(1)
  define_tokens += "#define " + token + " " + str(i) + "\n"
  tokens.append(token)

with open("token_codes.hpp", "w") as file:
  file.write(define_tokens)



print_func = "#pragma once\n#include \"token_codes.hpp\"\n#include <string>\n#include <iostream>\nstatic std::string token_code_print(size_t type){\n  switch(type){\n"
for i, m in enumerate(tokens,start=1):
  token = m
  print_func += "  case " + token + ": return \"" + token + "\";"  + " break;\n"

print_func += "  default: return \"Unknown token\"; break;\n  }\n}"

with open("token_codes_print.hpp", "w") as file:
  file.write(print_func)
