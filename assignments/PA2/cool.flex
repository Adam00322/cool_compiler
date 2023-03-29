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

static int comment_nested_num = 0;
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
TRUE            t(?i:rue)
FALSE           f(?i:alse)
LINECOMMENT  --.*

 /*
  keywords        "class" | "else" | "false" | "fi" | "if" | "in" | "inherits" | "isvoid" | "let" | "loop" | "pool" | "then" | "while" |
  "case" | "esac" | "new" | "of" | "not" | "true"
 */

DARROW          =>
ASSIGN          <-
LESSEQUAL       <=
%x COMMENTS
%x STRING
%option noyywrap

%%
 /* begin of rules */

 /* one line comment */
<INITIAL>{LINECOMMENT}   { }

 /* Nested comments */
<INITIAL,COMMENTS>"(*" {
  comment_nested_num++;
  BEGIN(COMMENTS);
}

<COMMENTS>"\n" {
  curr_lineno++;
}

<COMMENTS>"*)" {
  comment_nested_num--;
  if(comment_nested_num == 0) {
    BEGIN(INITIAL);
  }
}

<COMMENTS><<EOF>> {
  yylval.error_msg = "EOF in comment";
  BEGIN(INITIAL);
  return (ERROR);
}

<COMMENTS>[^\n(*)]* { }

<COMMENTS>[(*)] { }

<INITIAL>"*)" {
  yylval.error_msg = "Unmatched *)";
  return (ERROR);
}

 /* keywords */
<INITIAL>(?i:class)      {  return (CLASS);  }
<INITIAL>(?i:else)       {  return (ELSE);   }
<INITIAL>(?i:fi)         {  return (FI);     }
<INITIAL>(?i:if)         {  return (IF);     }
<INITIAL>(?i:in)         {  return (IN);     }
<INITIAL>(?i:inherits)   {  return (INHERITS);}
<INITIAL>(?i:isvoid)     {  return (ISVOID); }
<INITIAL>(?i:let)        {  return (LET);    }
<INITIAL>(?i:loop)       {  return (LOOP);   }
<INITIAL>(?i:pool)       {  return (POOL);   }
<INITIAL>(?i:then)       {  return (THEN);   }
<INITIAL>(?i:while)      {  return (WHILE);  }
<INITIAL>(?i:case)       {  return (CASE);   }
<INITIAL>(?i:esac)       {  return (ESAC);   }
<INITIAL>(?i:new)        {  return (NEW);    }
<INITIAL>(?i:of)         {  return (OF);     }
<INITIAL>(?i:not)        {  return (NOT);    }

 /* true */
<INITIAL>{TRUE} {
  cool_yylval.boolean = true;
  return (BOOL_CONST);
}

 /* false */
<INITIAL>{FALSE} {
  cool_yylval.boolean = false;
  return (BOOL_CONST);
}

<INITIAL>{NUMBER} {
  cool_yylval.symbol = inttable.add_string(yytext);
  return (INT_CONST);
}

<INITIAL>{TYPEID} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return (TYPEID);
}

<INITIAL>{OBJECTID} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return (OBJECTID);
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
  /* start of string */
<INITIAL>"\"" {
  BEGIN(STRING);
}

<STRING><<EOF>> {
  yylval.error_msg = "EOF in string constant";
  BEGIN(INITIAL);
  return (ERROR);
}

 /* any normal contents in string */
<STRING>[^\\\"\n]* {
  yymore();
}

 /* \ in string and followed by a \n denotes change line */
<STRING>"\\\n" {
  curr_lineno++;
  yymore();
}

<STRING>"\\\"" {
  yymore();
}

<STRING>"\\\\" {
  yymore();
}

<STRING>"\\" {
  yymore();
}

<STRING>"\n" {
  yylval.error_msg = "Unterminated string constant";
  curr_lineno++;
  BEGIN(INITIAL);
  return (ERROR);
}

 /* end of string */
<STRING>"\"" {
  std::string raw_str(yytext, yyleng);
  raw_str = raw_str.substr(0, raw_str.length()-1);
  std::string ret_str = "";
  size_t len = raw_str.size();
  for(size_t i=0; i<len; ++i) {
    if(raw_str[i] == '\0') {
      cool_yylval.error_msg = "String contains null character.";
      BEGIN(INITIAL);
      return (ERROR);
    }else if(raw_str[i] == '\\') {
      if(i+1<len) {
        i++;
        char c = raw_str[i];
        switch(c) {
          case 'b':
            ret_str += "\b";
            break;
          case 't':
            ret_str += "\t";
            break;
          case 'n':
            ret_str += "\n";
            break;
          case 'f':
            ret_str += "\f";
            break;
          default:
            ret_str += c;
            break;
        }
      }else{
        ret_str += raw_str[i];
      }
    }else{
      ret_str += raw_str[i];
    }
  }
  if(ret_str.size() >= MAX_STR_CONST) {
    cool_yylval.error_msg = "String constant too long";
    BEGIN(INITIAL);
    return (ERROR);
  }
  cool_yylval.symbol = stringtable.add_string((char *)ret_str.c_str());
  BEGIN(INITIAL);
  return STR_CONST;
}

 /* operators */
<INITIAL>{LESSEQUAL}    { return (LE); }
<INITIAL>{ASSIGN}       { return (ASSIGN); }
<INITIAL>{DARROW}       { return (DARROW); }
<INITIAL>"+"            { return int('+'); }
<INITIAL>"-"            { return int('-'); }
<INITIAL>"*"            { return int('*'); }
<INITIAL>"/"            { return int('/'); }
<INITIAL>"<"            { return int('<'); }
<INITIAL>"="            { return int('='); }
<INITIAL>"."            { return int('.'); }
<INITIAL>";"            { return int(';'); }
<INITIAL>"~"            { return int('~'); }
<INITIAL>"{"            { return int('{'); }
<INITIAL>"}"            { return int('}'); }
<INITIAL>"("            { return int('('); }
<INITIAL>")"            { return int(')'); }
<INITIAL>":"            { return int(':'); }
<INITIAL>"@"            { return int('@'); }
<INITIAL>","            { return int(','); }

 /* white space */
<INITIAL>[ \r\f\t\v]+ { }

 /* end of line */
<INITIAL>"\n" {
  curr_lineno++;
}

 /* if all are not match, then error */
<INITIAL>[^\n] {
  yylval.error_msg = yytext;
  return (ERROR);
}

%%
