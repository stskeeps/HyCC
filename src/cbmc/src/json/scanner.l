%{
// Strictly follows http://www.json.org/
%}

%option 8bit nodefault
%option nounput
%option noinput
%option nounistd
%option never-interactive

%option noyywrap

%{

#define PARSER json_parser

#include "json_parser.h"
#include "json_y.tab.hpp"

#include <util/pragma_wsign_compare.def>
#include <util/pragma_wnull_conversion.def>
#include <util/pragma_wdeprecated_register.def>

%}

string  \"\"|\"{chars}\"
chars   {char}+
char    [^\"]|\\\"

number  {int}|{int}{frac}|{int}{exp}|{int}{frac}{exp}
int     {digit}|{digit19}{digits}|-{digit}|-{digit19}{digits}
frac    "."{digits}
exp     {e}{digits}
digits  {digit}+
digit   [0-9]
digit19 [1-9]
e       e|e\+|e-|E|E\+|E- 

%%

{string}        { return TOK_STRING; }
{number}        { return TOK_NUMBER; }
"true"          { return TOK_TRUE; }
"false"         { return TOK_FALSE; }
"null"          { return TOK_NULL; }

[" "\r\n\t]     { /* eat */ }
.               { return yytext[0]; }

