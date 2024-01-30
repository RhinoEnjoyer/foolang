%{
  #include <stdio.h>
  #include <stdint.h>
  #include "../token/token_codes.hpp"
  #include "../token/token.hpp"

  #ifdef PRINT_TOKS_DEF
    #define PRINT_TOK(type) printf(type);printf(": \" %s \" \n", yytext)
  #else
    #define PRINT_TOK(type)
  #endif
%}

%option nounput yylineno
%option reentrant noyywrap

DIGIT       [0-9]
CHAR        [a-zA-Z_]
HEXLIT      0x[0-9a-fA-F]+
BINLIT      0b[01]+
STRLIT      \"(\\.|[^"\\])*\"
ID          {CHAR}({CHAR}|{DIGIT})*
/* PLZ FIX */
TEMPLATE    \$
POINTER     #
/* this one is fine*/
BUILTIN     @{ID}

TONE_INT ({DIGIT}+')+({DIGIT}+)
INT {DIGIT}+
FLOAT {INT}"."{INT}

%%

[ \t\n] {return WS;}

{STRLIT} {PRINT_TOK("string literal tok");return STRLIT;}
{HEXLIT} {PRINT_TOK("hex tok"); return HEX;}
{BINLIT} {PRINT_TOK("bin tok"); return BIN;}

{BUILTIN} {
  PRINT_TOK("builtin tok");
  return BUILTIN;
}
{ID} {PRINT_TOK("ID tok"); return ID;}

{FLOAT} {PRINT_TOK("float tok"); return FLOAT;}

{TONE_INT} {PRINT_TOK("tone int tok"); return TONE_INT;}
{INT} {PRINT_TOK("int tok"); return INT;}
{TEMPLATE} {PRINT_TOK("template tok"); return TEMPLATE;}
{POINTER} {PRINT_TOK("pointer tok"); return POINTER;}

"'"  {PRINT_TOK("tone tok"); return TONE;}
","  {PRINT_TOK("comma tok"); return COMMA;}
"."  {PRINT_TOK("dot tok"); return DOT;}
":"  {PRINT_TOK("colon tok"); return COLON;}

"==" {PRINT_TOK("equal tok"); return EQUAL;}
"!=" {PRINT_TOK("not equal tok"); return NEQUAL;}
">=" {PRINT_TOK("greater/equal tok"); return GEQUAL;}
"<=" {PRINT_TOK("less/equal tok"); return LEQUAL;}

"="  {PRINT_TOK("asign tok"); return ASIGN;}

">>"  {PRINT_TOK("right shift tok"); return SHIFTR;}
"<<"  {PRINT_TOK("left shift tok"); return SHIFTL;}

"&" {PRINT_TOK("and bin tok"); return AND;}
"\|" {PRINT_TOK("or bin tok"); return OR;}
"^+" {PRINT_TOK("xor bin tok"); return XOR;}
"!" {PRINT_TOK("not bin tok"); return NOT;}

"+" {PRINT_TOK("plus tok"); return PLUS;}
"-" {PRINT_TOK("minus tok"); return MINUS;}
"*" {PRINT_TOK("mod tok"); return MULT;}
"/" {PRINT_TOK("mod tok"); return DIV;}
"%" {PRINT_TOK("mod tok"); return MOD;}

">"  {PRINT_TOK("greater tok"); return GREATER;}
"<"  {PRINT_TOK("less tok"); return LESS;}

"{" {PRINT_TOK("left curly brace"); return LCBRACE;}
"}" {PRINT_TOK("right curly brace"); return RCBRACE;}

"[" {PRINT_TOK("left brace"); return LBRACE;}
"]" {PRINT_TOK("right brace"); return RBRACE;}

"(" {PRINT_TOK("left parenthesis"); return LPAREN;}
")" {PRINT_TOK("right parenthesis"); return RPAREN;}

";" {PRINT_TOK("semicolon tok"); return SEMICOLON;}


"//".* | "/*"([^*]|[*]+[^*/])*"*/"

. {PRINT_TOK("other tok"); return OTHER;}

<<EOF>> {PRINT_TOK("\nEnd of file\n"); return NULLTKN; }

%%
