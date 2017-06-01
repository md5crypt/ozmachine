/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "parser.y" /* yacc.c:1909  */

	#include <inttypes.h>
	#include "asm_struct.h"
	#define YY_DECL int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param , yyscan_t yyscanner, asm_context_t* context)

#line 50 "parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOKEN_INVALID = 258,
    TOKEN_KEYWORD_EXTERN = 259,
    TOKEN_KEYWORD_DEF = 260,
    TOKEN_KEYWORD_ACC = 261,
    TOKEN_CONST_ATOM = 262,
    TOKEN_CONST_VARNAME = 263,
    TOKEN_CONST_INTEGER = 264,
    TOKEN_REF_STACK = 265,
    TOKEN_REF_CLOSURE = 266,
    TOKEN_OP_ANY = 267,
    TOKEN_OP_FUN = 268,
    TOKEN_OP_REF = 269,
    TOKEN_OP_INT = 270,
    TOKEN_OP_CINT = 271,
    TOKEN_OP_INST = 272,
    TOKEN_OP_LABEL = 273,
    TOKEN_OP_KEY = 274,
    TOKEN_OP_MOVE = 275,
    TOKEN_OP_NOARG = 276,
    TOKEN_OP_BRANCH = 277,
    TOKEN_OP_PATTERN = 278,
    TOKEN_OP_RPN = 279,
    TOKEN_OP_RPNPUSH = 280,
    TOKEN_CONST_FLOAT = 281
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 17 "parser.y" /* yacc.c:1909  */
 
	asm_record_pair_t* v_recpair;
	asm_varible_t* v_varible;
	asm_refpair_t v_refpair;
	uint32_t v_int;
	double v_float;

#line 97 "parser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void* scanner, asm_context_t* context);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
