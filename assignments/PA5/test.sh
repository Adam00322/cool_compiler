#!/bin/sh
make cgen
../../bin/lexer $1 | ../../bin/parser | ../../bin/semant | ./cgen > my.s
../../bin/lexer $1 | ../../bin/parser | ../../bin/semant | ../../bin/cgen > expect.s
../../bin/spim -file ./my.s