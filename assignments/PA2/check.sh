#!/bin/bash
make clean
make lexer
if [ -e check_mine.txt ]; then 
  rm check_mine.txt 
fi
if [ -e check_expected.txt ]; then 
  rm check_expected.txt 
fi
./lexer $1 > check_mine.txt
../../bin/lexer $1 > check_expected.txt
result=$(diff check_mine.txt check_expected.txt)

if [ -z "$result" ]; then 
  echo "passed"
else 
  echo "$result"
fi