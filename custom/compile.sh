#!/bin/bash
OLVL="1"
GLVL="0"


printf "Generating tokens...\n"
cd ./parser/token
    ./tokens.py
cd - > /dev/null
printf "Done\n\n"

printf "Compiling parser...\n"
(
  cd ./parser/
    rm parser.so
    clang++ -g$GLVL -O$OLVL -fpic -shared -o parser.so parser.cpp
  cd - > /dev/null
) &

printf "Generating and Compiling lexer...\n"

(
  cd ./parser/lexer
  rm lexer.so
  #  flex -8 -f --header-file=lexer.tab.h --outfile=lexer.tab.c lexer.l
  flex -8 -L -X  --outfile=lexer.tab.c lexer.l
  clang++ -g$GLVL -O$OLVL  -fpic -shared -o lexer.so lexer.cpp
  cd - > /dev/null
) &

wait
printf "Done\n\n"


rm main

clang++ -g$GLVL -O$OLVL -o main main.cpp ./parser/parser.so ./parser/lexer/lexer.so

