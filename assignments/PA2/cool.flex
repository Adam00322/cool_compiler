 /*
 * The scanner definition for COOL.
 */

 /* 
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 *  to the code in the file.  Dont remove anything that was here initially
 */

%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

static int comment_line_num = 0;
%}
 /* begin of definitions */
 /*
 * Define names for regular expressions here.
 */
DIGIT           [0-9]
LETTER          [a-zA-Z]
TYPEID          [A-Z][a-zA-Z0-9_]*
OBJECTID        [a-z][a-zA-Z0-9_]*
NUMBER          {DIGIT}+

 /*
  keywords        "class" | "else" | "false" | "fi" | "if" | "in" | "inherits" | "isvoid" | "let" | "loop" | "pool" | "then" | "while" |
  "case" | "esac" | "new" | "of" | "not" | "true"
 */

DARROW          =>
ASSIGN          <-
LESSEQUAL       <=
%x COMMENTS
%x STRING

%%
 /* begin of rules */
 /*
  *  Nested comments
  */


 /*
  *  The multiple-character operators.
  */


 /* keywords */
(?i:class)      {  return (CLASS);  }
(?i:else)       {  return (ELSE);   }
(?i:fi)         {  return (FI);     }
(?i:if)         {  return (IF);     }
(?i:in)         {  return (IN);     }
(?i:inherits)   {  return (INHERITS);}
(?i:isvoid)     {  return (ISVOID); }
(?i:let)        {  return (LET);    }
(?i:loop)       {  return (LOOP);   }
(?i:pool)       {  return (POOL);   }
(?i:then)       {  return (THEN);   }
(?i:while)      {  return (WHILE);  }
(?i:case)       {  return (CASE);   }
(?i:esac)       {  return (ESAC);   }
(?i:new)        {  return (NEW);    }
(?i:of)         {  return (OF);     }
(?i:not)        {  return (NOT);    }

t(?i:rue) {
  cool_yylval.boolean = true;
  return (BOOL_CONST);
}

f(?i:alse) {
  cool_yylval.boolean = false;
  return (BOOL_CONST);
}

{NUMBER} {
  cool_yylval.symbol = inttable.add_string(yytext);
  return (INT_CONST);
}

{TYPEID} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return (TYPEID);
}

{OBJECTID} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return (OBJECTID);
}


{DARROW}		    { return (DARROW); }


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */



 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

 /* if all are not match, then error */
[^\n] {
  yylval.error_msg = yytext;
  return (ERROR);
}

%%
