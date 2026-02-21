/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 19 "C:/src/tmux/cmd-parse.y"


#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#ifndef _WIN32
#include <pwd.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <wchar.h>

#include "tmux.h"

static int			 yylex(void);
static int			 yyparse(void);
static void printflike(1,2)	 yyerror(const char *, ...);

static char			*yylex_token(int);
static char			*yylex_format(void);

struct cmd_parse_scope {
	int				 flag;
	TAILQ_ENTRY (cmd_parse_scope)	 entry;
};

enum cmd_parse_argument_type {
	CMD_PARSE_STRING,
	CMD_PARSE_COMMANDS,
	CMD_PARSE_PARSED_COMMANDS
};

struct cmd_parse_argument {
	enum cmd_parse_argument_type	 type;
	char				*string;
	struct cmd_parse_commands	*commands;
	struct cmd_list			*cmdlist;

	TAILQ_ENTRY(cmd_parse_argument)	 entry;
};
TAILQ_HEAD(cmd_parse_arguments, cmd_parse_argument);

struct cmd_parse_command {
	u_int				 line;
	struct cmd_parse_arguments	 arguments;

	TAILQ_ENTRY(cmd_parse_command)	 entry;
};
TAILQ_HEAD(cmd_parse_commands, cmd_parse_command);

struct cmd_parse_state {
	FILE				*f;

	const char			*buf;
	size_t				 len;
	size_t				 off;

	int				 condition;
	int				 eol;
	int				 eof;
	struct cmd_parse_input		*input;
	u_int				 escapes;

	char				*error;
	struct cmd_parse_commands	*commands;

	struct cmd_parse_scope		*scope;
	TAILQ_HEAD(, cmd_parse_scope)	 stack;
};
static struct cmd_parse_state parse_state;

static char	*cmd_parse_get_error(const char *, u_int, const char *);
static void	 cmd_parse_free_command(struct cmd_parse_command *);
static struct cmd_parse_commands *cmd_parse_new_commands(void);
static void	 cmd_parse_free_commands(struct cmd_parse_commands *);
static void	 cmd_parse_build_commands(struct cmd_parse_commands *,
		     struct cmd_parse_input *, struct cmd_parse_result *);
static void	 cmd_parse_print_commands(struct cmd_parse_input *,
		     struct cmd_list *);


#line 157 "C:/src/tmux/cmd-parse.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ERROR = 258,                   /* ERROR  */
    HIDDEN = 259,                  /* HIDDEN  */
    IF = 260,                      /* IF  */
    ELSE = 261,                    /* ELSE  */
    ELIF = 262,                    /* ELIF  */
    ENDIF = 263,                   /* ENDIF  */
    FORMAT = 264,                  /* FORMAT  */
    TOKEN = 265,                   /* TOKEN  */
    EQUALS = 266                   /* EQUALS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 106 "C:/src/tmux/cmd-parse.y"

	char					 *token;
	struct cmd_parse_arguments		 *arguments;
	struct cmd_parse_argument		 *argument;
	int					  flag;
	struct {
		int				  flag;
		struct cmd_parse_commands	 *commands;
	} elif;
	struct cmd_parse_commands		 *commands;
	struct cmd_parse_command		 *command;

#line 228 "C:/src/tmux/cmd-parse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ERROR = 3,                      /* ERROR  */
  YYSYMBOL_HIDDEN = 4,                     /* HIDDEN  */
  YYSYMBOL_IF = 5,                         /* IF  */
  YYSYMBOL_ELSE = 6,                       /* ELSE  */
  YYSYMBOL_ELIF = 7,                       /* ELIF  */
  YYSYMBOL_ENDIF = 8,                      /* ENDIF  */
  YYSYMBOL_FORMAT = 9,                     /* FORMAT  */
  YYSYMBOL_TOKEN = 10,                     /* TOKEN  */
  YYSYMBOL_EQUALS = 11,                    /* EQUALS  */
  YYSYMBOL_12_n_ = 12,                     /* '\n'  */
  YYSYMBOL_13_ = 13,                       /* ';'  */
  YYSYMBOL_14_ = 14,                       /* '{'  */
  YYSYMBOL_15_ = 15,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 16,                  /* $accept  */
  YYSYMBOL_lines = 17,                     /* lines  */
  YYSYMBOL_statements = 18,                /* statements  */
  YYSYMBOL_statement = 19,                 /* statement  */
  YYSYMBOL_format = 20,                    /* format  */
  YYSYMBOL_expanded = 21,                  /* expanded  */
  YYSYMBOL_optional_assignment = 22,       /* optional_assignment  */
  YYSYMBOL_assignment = 23,                /* assignment  */
  YYSYMBOL_hidden_assignment = 24,         /* hidden_assignment  */
  YYSYMBOL_if_open = 25,                   /* if_open  */
  YYSYMBOL_if_else = 26,                   /* if_else  */
  YYSYMBOL_if_elif = 27,                   /* if_elif  */
  YYSYMBOL_if_close = 28,                  /* if_close  */
  YYSYMBOL_condition = 29,                 /* condition  */
  YYSYMBOL_elif = 30,                      /* elif  */
  YYSYMBOL_commands = 31,                  /* commands  */
  YYSYMBOL_command = 32,                   /* command  */
  YYSYMBOL_condition1 = 33,                /* condition1  */
  YYSYMBOL_elif1 = 34,                     /* elif1  */
  YYSYMBOL_arguments = 35,                 /* arguments  */
  YYSYMBOL_argument = 36,                  /* argument  */
  YYSYMBOL_argument_statements = 37        /* argument_statements  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   104

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  16
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  47
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  75

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   266


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      12,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    13,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    14,     2,    15,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   138,   138,   139,   146,   150,   158,   162,   167,   178,
     190,   194,   199,   223,   224,   226,   244,   262,   276,   288,
     301,   311,   320,   330,   344,   361,   372,   390,   401,   405,
     411,   425,   430,   438,   452,   470,   479,   489,   503,   520,
     531,   549,   556,   562,   568,   574,   581,   585
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ERROR", "HIDDEN",
  "IF", "ELSE", "ELIF", "ENDIF", "FORMAT", "TOKEN", "EQUALS", "'\\n'",
  "';'", "'{'", "'}'", "$accept", "lines", "statements", "statement",
  "format", "expanded", "optional_assignment", "assignment",
  "hidden_assignment", "if_open", "if_else", "if_elif", "if_close",
  "condition", "elif", "commands", "command", "condition1", "elif1",
  "arguments", "argument", "argument_statements", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-32)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-15)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      49,     3,    14,   -32,    33,    49,    29,    47,    60,   -32,
       4,   -32,    35,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
     -32,    68,   -32,    83,    81,    38,     5,    17,   -32,   -32,
     -32,    81,   -32,    83,    71,   -32,    14,   -32,    38,    38,
     -32,    -1,   -32,   -32,    81,    40,   -32,   -32,    76,    86,
     -32,    -1,   -32,    32,    58,    38,   -32,    84,   -32,    81,
      81,    88,   -32,   -32,   -32,    32,   -32,    79,    62,    81,
     -32,   -32,   -32,    79,   -32
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     0,    15,     0,     3,     0,     0,    32,     7,
      13,     8,     9,    27,    31,    16,    10,    11,    12,    17,
       1,     0,     4,    33,     6,    13,     0,    28,     5,    43,
      44,     6,    34,    41,     6,    18,     0,    20,    13,    13,
      35,     0,    30,    29,     6,     0,    45,    42,     0,     0,
      21,     0,    19,     0,    39,    13,    37,     0,    46,     6,
       6,     0,    23,    36,    40,     0,    47,     6,    25,     6,
      38,    22,    26,     6,    24
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -32,   -32,   -23,    -5,   -32,    59,   -32,   -32,   -32,    -8,
     -31,   -30,    -9,   -32,   -18,    -4,    74,    75,    50,    70,
     -32,   -32
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     4,     5,     6,    18,    19,     7,     8,     9,    10,
      38,    39,    40,    11,    51,    12,    13,    14,    41,    32,
      33,    46
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      21,    34,    25,    48,    49,    35,    26,    37,    44,     2,
      55,    35,    36,    37,    15,     3,    24,    25,    27,    25,
      61,    26,     2,    16,    17,    50,    45,   -13,     3,    21,
      25,    25,    56,    20,    53,    54,    67,    68,    49,    57,
      37,    22,    62,     2,    63,    27,    73,    25,    27,     3,
      72,    65,    22,     1,     2,    58,    70,    23,    71,   -13,
       3,    -6,    21,    21,    74,    36,     1,     2,    21,    36,
     -14,    27,   -13,     3,    -6,     1,     2,    35,    36,    37,
      28,   -13,     3,     1,     2,     1,     2,    37,    59,   -13,
       3,   -13,     3,    29,    30,    52,    28,    31,    60,    66,
      69,    42,    43,    47,    64
};

static const yytype_int8 yycheck[] =
{
       5,    24,    10,    34,    34,     6,    10,     8,    31,     5,
      41,     6,     7,     8,    11,    11,    12,    25,    13,    27,
      51,    25,     5,     9,    10,    34,    31,    10,    11,    34,
      38,    39,    41,     0,    38,    39,    59,    60,    68,    44,
       8,    12,    51,     5,    53,    13,    69,    55,    13,    11,
      68,    55,    12,     4,     5,    15,    65,    10,    67,    10,
      11,    12,    67,    68,    73,     7,     4,     5,    73,     7,
      10,    13,    10,    11,    12,     4,     5,     6,     7,     8,
      12,    10,    11,     4,     5,     4,     5,     8,    12,    10,
      11,    10,    11,    10,    11,    36,    12,    14,    12,    15,
      12,    27,    27,    33,    54
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     4,     5,    11,    17,    18,    19,    22,    23,    24,
      25,    29,    31,    32,    33,    11,     9,    10,    20,    21,
       0,    19,    12,    10,    12,    25,    31,    13,    12,    10,
      11,    14,    35,    36,    18,     6,     7,     8,    26,    27,
      28,    34,    32,    33,    18,    19,    37,    35,    26,    27,
      28,    30,    21,    31,    31,    26,    28,    19,    15,    12,
      12,    26,    28,    28,    34,    31,    15,    18,    18,    12,
      28,    28,    30,    18,    28
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    16,    17,    17,    18,    18,    19,    19,    19,    19,
      20,    20,    21,    22,    22,    23,    24,    25,    26,    27,
      28,    29,    29,    29,    29,    30,    30,    31,    31,    31,
      31,    31,    32,    32,    32,    33,    33,    33,    33,    34,
      34,    35,    35,    36,    36,    36,    37,    37
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     2,     3,     0,     1,     1,     1,
       1,     1,     1,     0,     1,     1,     2,     2,     1,     2,
       1,     4,     7,     5,     8,     3,     4,     1,     2,     3,
       3,     1,     1,     2,     3,     3,     5,     4,     6,     2,
       3,     1,     2,     1,     1,     2,     2,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* lines: statements  */
#line 140 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			ps->commands = (yyvsp[0].commands);
		}
#line 1297 "C:/src/tmux/cmd-parse.c"
    break;

  case 4: /* statements: statement '\n'  */
#line 147 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = (yyvsp[-1].commands);
		}
#line 1305 "C:/src/tmux/cmd-parse.c"
    break;

  case 5: /* statements: statements statement '\n'  */
#line 151 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = (yyvsp[-2].commands);
			TAILQ_CONCAT((yyval.commands), (yyvsp[-1].commands), entry);
			free((yyvsp[-1].commands));
		}
#line 1315 "C:/src/tmux/cmd-parse.c"
    break;

  case 6: /* statement: %empty  */
#line 158 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = xmalloc (sizeof *(yyval.commands));
			TAILQ_INIT((yyval.commands));
		}
#line 1324 "C:/src/tmux/cmd-parse.c"
    break;

  case 7: /* statement: hidden_assignment  */
#line 163 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = xmalloc (sizeof *(yyval.commands));
			TAILQ_INIT((yyval.commands));
		}
#line 1333 "C:/src/tmux/cmd-parse.c"
    break;

  case 8: /* statement: condition  */
#line 168 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			if (ps->scope == NULL || ps->scope->flag)
				(yyval.commands) = (yyvsp[0].commands);
			else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[0].commands));
			}
		}
#line 1348 "C:/src/tmux/cmd-parse.c"
    break;

  case 9: /* statement: commands  */
#line 179 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			if (ps->scope == NULL || ps->scope->flag)
				(yyval.commands) = (yyvsp[0].commands);
			else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[0].commands));
			}
		}
#line 1363 "C:/src/tmux/cmd-parse.c"
    break;

  case 10: /* format: FORMAT  */
#line 191 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.token) = (yyvsp[0].token);
		}
#line 1371 "C:/src/tmux/cmd-parse.c"
    break;

  case 11: /* format: TOKEN  */
#line 195 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.token) = (yyvsp[0].token);
		}
#line 1379 "C:/src/tmux/cmd-parse.c"
    break;

  case 12: /* expanded: format  */
#line 200 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;
			struct cmd_parse_input	*pi = ps->input;
			struct format_tree	*ft;
			struct client		*c = pi->c;
			struct cmd_find_state	*fsp;
			struct cmd_find_state	 fs;
			int			 flags = FORMAT_NOJOBS;

			if (cmd_find_valid_state(&pi->fs))
				fsp = &pi->fs;
			else {
				cmd_find_from_client(&fs, c, 0);
				fsp = &fs;
			}
			ft = format_create(NULL, pi->item, FORMAT_NONE, flags);
			format_defaults(ft, c, fsp->s, fsp->wl, fsp->wp);

			(yyval.token) = format_expand(ft, (yyvsp[0].token));
			format_free(ft);
			free((yyvsp[0].token));
		}
#line 1406 "C:/src/tmux/cmd-parse.c"
    break;

  case 15: /* assignment: EQUALS  */
#line 227 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;
			int			 flags = ps->input->flags;
			int			 flag = 1;
			struct cmd_parse_scope	*scope;

			if (ps->scope != NULL) {
				flag = ps->scope->flag;
				TAILQ_FOREACH(scope, &ps->stack, entry)
					flag = flag && scope->flag;
			}

			if ((~flags & CMD_PARSE_PARSEONLY) && flag)
				environ_put(global_environ, (yyvsp[0].token), 0);
			free((yyvsp[0].token));
		}
#line 1427 "C:/src/tmux/cmd-parse.c"
    break;

  case 16: /* hidden_assignment: HIDDEN EQUALS  */
#line 245 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;
			int			 flags = ps->input->flags;
			int			 flag = 1;
			struct cmd_parse_scope	*scope;

			if (ps->scope != NULL) {
				flag = ps->scope->flag;
				TAILQ_FOREACH(scope, &ps->stack, entry)
					flag = flag && scope->flag;
			}

			if ((~flags & CMD_PARSE_PARSEONLY) && flag)
				environ_put(global_environ, (yyvsp[0].token), ENVIRON_HIDDEN);
			free((yyvsp[0].token));
		}
#line 1448 "C:/src/tmux/cmd-parse.c"
    break;

  case 17: /* if_open: IF expanded  */
#line 263 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;
			struct cmd_parse_scope	*scope;

			scope = xmalloc(sizeof *scope);
			(yyval.flag) = scope->flag = format_true((yyvsp[0].token));
			free((yyvsp[0].token));

			if (ps->scope != NULL)
				TAILQ_INSERT_HEAD(&ps->stack, ps->scope, entry);
			ps->scope = scope;
		}
#line 1465 "C:/src/tmux/cmd-parse.c"
    break;

  case 18: /* if_else: ELSE  */
#line 277 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;
			struct cmd_parse_scope	*scope;

			scope = xmalloc(sizeof *scope);
			scope->flag = !ps->scope->flag;

			free(ps->scope);
			ps->scope = scope;
		}
#line 1480 "C:/src/tmux/cmd-parse.c"
    break;

  case 19: /* if_elif: ELIF expanded  */
#line 289 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;
			struct cmd_parse_scope	*scope;

			scope = xmalloc(sizeof *scope);
			(yyval.flag) = scope->flag = format_true((yyvsp[0].token));
			free((yyvsp[0].token));

			free(ps->scope);
			ps->scope = scope;
		}
#line 1496 "C:/src/tmux/cmd-parse.c"
    break;

  case 20: /* if_close: ENDIF  */
#line 302 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			free(ps->scope);
			ps->scope = TAILQ_FIRST(&ps->stack);
			if (ps->scope != NULL)
				TAILQ_REMOVE(&ps->stack, ps->scope, entry);
		}
#line 1509 "C:/src/tmux/cmd-parse.c"
    break;

  case 21: /* condition: if_open '\n' statements if_close  */
#line 312 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-3].flag))
				(yyval.commands) = (yyvsp[-1].commands);
			else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-1].commands));
			}
		}
#line 1522 "C:/src/tmux/cmd-parse.c"
    break;

  case 22: /* condition: if_open '\n' statements if_else '\n' statements if_close  */
#line 321 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-6].flag)) {
				(yyval.commands) = (yyvsp[-4].commands);
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else {
				(yyval.commands) = (yyvsp[-1].commands);
				cmd_parse_free_commands((yyvsp[-4].commands));
			}
		}
#line 1536 "C:/src/tmux/cmd-parse.c"
    break;

  case 23: /* condition: if_open '\n' statements elif if_close  */
#line 331 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-4].flag)) {
				(yyval.commands) = (yyvsp[-2].commands);
				cmd_parse_free_commands((yyvsp[-1].elif).commands);
			} else if ((yyvsp[-1].elif).flag) {
				(yyval.commands) = (yyvsp[-1].elif).commands;
				cmd_parse_free_commands((yyvsp[-2].commands));
			} else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-2].commands));
				cmd_parse_free_commands((yyvsp[-1].elif).commands);
			}
		}
#line 1554 "C:/src/tmux/cmd-parse.c"
    break;

  case 24: /* condition: if_open '\n' statements elif if_else '\n' statements if_close  */
#line 345 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-7].flag)) {
				(yyval.commands) = (yyvsp[-5].commands);
				cmd_parse_free_commands((yyvsp[-4].elif).commands);
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else if ((yyvsp[-4].elif).flag) {
				(yyval.commands) = (yyvsp[-4].elif).commands;
				cmd_parse_free_commands((yyvsp[-5].commands));
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else {
				(yyval.commands) = (yyvsp[-1].commands);
				cmd_parse_free_commands((yyvsp[-5].commands));
				cmd_parse_free_commands((yyvsp[-4].elif).commands);
			}
		}
#line 1574 "C:/src/tmux/cmd-parse.c"
    break;

  case 25: /* elif: if_elif '\n' statements  */
#line 362 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-2].flag)) {
				(yyval.elif).flag = 1;
				(yyval.elif).commands = (yyvsp[0].commands);
			} else {
				(yyval.elif).flag = 0;
				(yyval.elif).commands = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[0].commands));
			}
		}
#line 1589 "C:/src/tmux/cmd-parse.c"
    break;

  case 26: /* elif: if_elif '\n' statements elif  */
#line 373 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-3].flag)) {
				(yyval.elif).flag = 1;
				(yyval.elif).commands = (yyvsp[-1].commands);
				cmd_parse_free_commands((yyvsp[0].elif).commands);
			} else if ((yyvsp[0].elif).flag) {
				(yyval.elif).flag = 1;
				(yyval.elif).commands = (yyvsp[0].elif).commands;
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else {
				(yyval.elif).flag = 0;
				(yyval.elif).commands = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-1].commands));
				cmd_parse_free_commands((yyvsp[0].elif).commands);
			}
		}
#line 1610 "C:/src/tmux/cmd-parse.c"
    break;

  case 27: /* commands: command  */
#line 391 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			(yyval.commands) = cmd_parse_new_commands();
			if (!TAILQ_EMPTY(&(yyvsp[0].command)->arguments) &&
			    (ps->scope == NULL || ps->scope->flag))
				TAILQ_INSERT_TAIL((yyval.commands), (yyvsp[0].command), entry);
			else
				cmd_parse_free_command((yyvsp[0].command));
		}
#line 1625 "C:/src/tmux/cmd-parse.c"
    break;

  case 28: /* commands: commands ';'  */
#line 402 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = (yyvsp[-1].commands);
		}
#line 1633 "C:/src/tmux/cmd-parse.c"
    break;

  case 29: /* commands: commands ';' condition1  */
#line 406 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = (yyvsp[-2].commands);
			TAILQ_CONCAT((yyval.commands), (yyvsp[0].commands), entry);
			free((yyvsp[0].commands));
		}
#line 1643 "C:/src/tmux/cmd-parse.c"
    break;

  case 30: /* commands: commands ';' command  */
#line 412 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			if (!TAILQ_EMPTY(&(yyvsp[0].command)->arguments) &&
			    (ps->scope == NULL || ps->scope->flag)) {
				(yyval.commands) = (yyvsp[-2].commands);
				TAILQ_INSERT_TAIL((yyval.commands), (yyvsp[0].command), entry);
			} else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-2].commands));
				cmd_parse_free_command((yyvsp[0].command));
			}
		}
#line 1661 "C:/src/tmux/cmd-parse.c"
    break;

  case 31: /* commands: condition1  */
#line 426 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.commands) = (yyvsp[0].commands);
		}
#line 1669 "C:/src/tmux/cmd-parse.c"
    break;

  case 32: /* command: assignment  */
#line 431 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state	*ps = &parse_state;

			(yyval.command) = xcalloc(1, sizeof *(yyval.command));
			(yyval.command)->line = ps->input->line;
			TAILQ_INIT(&(yyval.command)->arguments);
		}
#line 1681 "C:/src/tmux/cmd-parse.c"
    break;

  case 33: /* command: optional_assignment TOKEN  */
#line 439 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state		*ps = &parse_state;
			struct cmd_parse_argument	*arg;

			(yyval.command) = xcalloc(1, sizeof *(yyval.command));
			(yyval.command)->line = ps->input->line;
			TAILQ_INIT(&(yyval.command)->arguments);

			arg = xcalloc(1, sizeof *arg);
			arg->type = CMD_PARSE_STRING;
			arg->string = (yyvsp[0].token);
			TAILQ_INSERT_HEAD(&(yyval.command)->arguments, arg, entry);
		}
#line 1699 "C:/src/tmux/cmd-parse.c"
    break;

  case 34: /* command: optional_assignment TOKEN arguments  */
#line 453 "C:/src/tmux/cmd-parse.y"
                {
			struct cmd_parse_state		*ps = &parse_state;
			struct cmd_parse_argument	*arg;

			(yyval.command) = xcalloc(1, sizeof *(yyval.command));
			(yyval.command)->line = ps->input->line;
			TAILQ_INIT(&(yyval.command)->arguments);

			TAILQ_CONCAT(&(yyval.command)->arguments, (yyvsp[0].arguments), entry);
			free((yyvsp[0].arguments));

			arg = xcalloc(1, sizeof *arg);
			arg->type = CMD_PARSE_STRING;
			arg->string = (yyvsp[-1].token);
			TAILQ_INSERT_HEAD(&(yyval.command)->arguments, arg, entry);
		}
#line 1720 "C:/src/tmux/cmd-parse.c"
    break;

  case 35: /* condition1: if_open commands if_close  */
#line 471 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-2].flag))
				(yyval.commands) = (yyvsp[-1].commands);
			else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-1].commands));
			}
		}
#line 1733 "C:/src/tmux/cmd-parse.c"
    break;

  case 36: /* condition1: if_open commands if_else commands if_close  */
#line 480 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-4].flag)) {
				(yyval.commands) = (yyvsp[-3].commands);
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else {
				(yyval.commands) = (yyvsp[-1].commands);
				cmd_parse_free_commands((yyvsp[-3].commands));
			}
		}
#line 1747 "C:/src/tmux/cmd-parse.c"
    break;

  case 37: /* condition1: if_open commands elif1 if_close  */
#line 490 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-3].flag)) {
				(yyval.commands) = (yyvsp[-2].commands);
				cmd_parse_free_commands((yyvsp[-1].elif).commands);
			} else if ((yyvsp[-1].elif).flag) {
				(yyval.commands) = (yyvsp[-1].elif).commands;
				cmd_parse_free_commands((yyvsp[-2].commands));
			} else {
				(yyval.commands) = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-2].commands));
				cmd_parse_free_commands((yyvsp[-1].elif).commands);
			}
		}
#line 1765 "C:/src/tmux/cmd-parse.c"
    break;

  case 38: /* condition1: if_open commands elif1 if_else commands if_close  */
#line 504 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-5].flag)) {
				(yyval.commands) = (yyvsp[-4].commands);
				cmd_parse_free_commands((yyvsp[-3].elif).commands);
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else if ((yyvsp[-3].elif).flag) {
				(yyval.commands) = (yyvsp[-3].elif).commands;
				cmd_parse_free_commands((yyvsp[-4].commands));
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else {
				(yyval.commands) = (yyvsp[-1].commands);
				cmd_parse_free_commands((yyvsp[-4].commands));
				cmd_parse_free_commands((yyvsp[-3].elif).commands);
			}
		}
#line 1785 "C:/src/tmux/cmd-parse.c"
    break;

  case 39: /* elif1: if_elif commands  */
#line 521 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-1].flag)) {
				(yyval.elif).flag = 1;
				(yyval.elif).commands = (yyvsp[0].commands);
			} else {
				(yyval.elif).flag = 0;
				(yyval.elif).commands = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[0].commands));
			}
		}
#line 1800 "C:/src/tmux/cmd-parse.c"
    break;

  case 40: /* elif1: if_elif commands elif1  */
#line 532 "C:/src/tmux/cmd-parse.y"
                {
			if ((yyvsp[-2].flag)) {
				(yyval.elif).flag = 1;
				(yyval.elif).commands = (yyvsp[-1].commands);
				cmd_parse_free_commands((yyvsp[0].elif).commands);
			} else if ((yyvsp[0].elif).flag) {
				(yyval.elif).flag = 1;
				(yyval.elif).commands = (yyvsp[0].elif).commands;
				cmd_parse_free_commands((yyvsp[-1].commands));
			} else {
				(yyval.elif).flag = 0;
				(yyval.elif).commands = cmd_parse_new_commands();
				cmd_parse_free_commands((yyvsp[-1].commands));
				cmd_parse_free_commands((yyvsp[0].elif).commands);
			}
		}
#line 1821 "C:/src/tmux/cmd-parse.c"
    break;

  case 41: /* arguments: argument  */
#line 550 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.arguments) = xcalloc(1, sizeof *(yyval.arguments));
			TAILQ_INIT((yyval.arguments));

			TAILQ_INSERT_HEAD((yyval.arguments), (yyvsp[0].argument), entry);
		}
#line 1832 "C:/src/tmux/cmd-parse.c"
    break;

  case 42: /* arguments: argument arguments  */
#line 557 "C:/src/tmux/cmd-parse.y"
                {
			TAILQ_INSERT_HEAD((yyvsp[0].arguments), (yyvsp[-1].argument), entry);
			(yyval.arguments) = (yyvsp[0].arguments);
		}
#line 1841 "C:/src/tmux/cmd-parse.c"
    break;

  case 43: /* argument: TOKEN  */
#line 563 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.argument) = xcalloc(1, sizeof *(yyval.argument));
			(yyval.argument)->type = CMD_PARSE_STRING;
			(yyval.argument)->string = (yyvsp[0].token);
		}
#line 1851 "C:/src/tmux/cmd-parse.c"
    break;

  case 44: /* argument: EQUALS  */
#line 569 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.argument) = xcalloc(1, sizeof *(yyval.argument));
			(yyval.argument)->type = CMD_PARSE_STRING;
			(yyval.argument)->string = (yyvsp[0].token);
		}
#line 1861 "C:/src/tmux/cmd-parse.c"
    break;

  case 45: /* argument: '{' argument_statements  */
#line 575 "C:/src/tmux/cmd-parse.y"
                {
			(yyval.argument) = xcalloc(1, sizeof *(yyval.argument));
			(yyval.argument)->type = CMD_PARSE_COMMANDS;
			(yyval.argument)->commands = (yyvsp[0].commands);
		}
#line 1871 "C:/src/tmux/cmd-parse.c"
    break;

  case 46: /* argument_statements: statement '}'  */
#line 582 "C:/src/tmux/cmd-parse.y"
                        {
				(yyval.commands) = (yyvsp[-1].commands);
			}
#line 1879 "C:/src/tmux/cmd-parse.c"
    break;

  case 47: /* argument_statements: statements statement '}'  */
#line 586 "C:/src/tmux/cmd-parse.y"
                        {
				(yyval.commands) = (yyvsp[-2].commands);
				TAILQ_CONCAT((yyval.commands), (yyvsp[-1].commands), entry);
				free((yyvsp[-1].commands));
			}
#line 1889 "C:/src/tmux/cmd-parse.c"
    break;


#line 1893 "C:/src/tmux/cmd-parse.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 592 "C:/src/tmux/cmd-parse.y"


static char *
cmd_parse_get_error(const char *file, u_int line, const char *error)
{
	char	*s;

	if (file == NULL)
		s = xstrdup(error);
	else
		xasprintf(&s, "%s:%u: %s", file, line, error);
	return (s);
}

static void
cmd_parse_print_commands(struct cmd_parse_input *pi, struct cmd_list *cmdlist)
{
	char	*s;

	if (pi->item == NULL || (~pi->flags & CMD_PARSE_VERBOSE))
		return;
	s = cmd_list_print(cmdlist, 0);
	if (pi->file != NULL)
		cmdq_print(pi->item, "%s:%u: %s", pi->file, pi->line, s);
	else
		cmdq_print(pi->item, "%u: %s", pi->line, s);
	free(s);
}

static void
cmd_parse_free_argument(struct cmd_parse_argument *arg)
{
	switch (arg->type) {
	case CMD_PARSE_STRING:
		free(arg->string);
		break;
	case CMD_PARSE_COMMANDS:
		cmd_parse_free_commands(arg->commands);
		break;
	case CMD_PARSE_PARSED_COMMANDS:
		cmd_list_free(arg->cmdlist);
		break;
	}
	free(arg);
}

static void
cmd_parse_free_arguments(struct cmd_parse_arguments *args)
{
	struct cmd_parse_argument	*arg, *arg1;

	TAILQ_FOREACH_SAFE(arg, args, entry, arg1) {
		TAILQ_REMOVE(args, arg, entry);
		cmd_parse_free_argument(arg);
	}
}

static void
cmd_parse_free_command(struct cmd_parse_command *cmd)
{
	cmd_parse_free_arguments(&cmd->arguments);
	free(cmd);
}

static struct cmd_parse_commands *
cmd_parse_new_commands(void)
{
	struct cmd_parse_commands	*cmds;

	cmds = xmalloc(sizeof *cmds);
	TAILQ_INIT(cmds);
	return (cmds);
}

static void
cmd_parse_free_commands(struct cmd_parse_commands *cmds)
{
	struct cmd_parse_command	*cmd, *cmd1;

	TAILQ_FOREACH_SAFE(cmd, cmds, entry, cmd1) {
		TAILQ_REMOVE(cmds, cmd, entry);
		cmd_parse_free_command(cmd);
	}
	free(cmds);
}

static struct cmd_parse_commands *
cmd_parse_run_parser(char **cause)
{
	struct cmd_parse_state	*ps = &parse_state;
	struct cmd_parse_scope	*scope, *scope1;
	int			 retval;

	ps->commands = NULL;
	TAILQ_INIT(&ps->stack);

	retval = yyparse();
	TAILQ_FOREACH_SAFE(scope, &ps->stack, entry, scope1) {
		TAILQ_REMOVE(&ps->stack, scope, entry);
		free(scope);
	}
	if (retval != 0) {
		*cause = ps->error;
		return (NULL);
	}

	if (ps->commands == NULL)
		return (cmd_parse_new_commands());
	return (ps->commands);
}

static struct cmd_parse_commands *
cmd_parse_do_file(FILE *f, struct cmd_parse_input *pi, char **cause)
{
	struct cmd_parse_state	*ps = &parse_state;

	memset(ps, 0, sizeof *ps);
	ps->input = pi;
	ps->f = f;
	return (cmd_parse_run_parser(cause));
}

static struct cmd_parse_commands *
cmd_parse_do_buffer(const char *buf, size_t len, struct cmd_parse_input *pi,
    char **cause)
{
	struct cmd_parse_state	*ps = &parse_state;

	memset(ps, 0, sizeof *ps);
	ps->input = pi;
	ps->buf = buf;
	ps->len = len;
	return (cmd_parse_run_parser(cause));
}

static void
cmd_parse_log_commands(struct cmd_parse_commands *cmds, const char *prefix)
{
	struct cmd_parse_command	*cmd;
	struct cmd_parse_argument	*arg;
	u_int				 i, j;
	char				*s;

	i = 0;
	TAILQ_FOREACH(cmd, cmds, entry) {
		j = 0;
		TAILQ_FOREACH(arg, &cmd->arguments, entry) {
			switch (arg->type) {
			case CMD_PARSE_STRING:
				log_debug("%s %u:%u: %s", prefix, i, j,
				    arg->string);
				break;
			case CMD_PARSE_COMMANDS:
				xasprintf(&s, "%s %u:%u", prefix, i, j);
				cmd_parse_log_commands(arg->commands, s);
				free(s);
				break;
			case CMD_PARSE_PARSED_COMMANDS:
				s = cmd_list_print(arg->cmdlist, 0);
				log_debug("%s %u:%u: %s", prefix, i, j, s);
				free(s);
				break;
			}
			j++;
		}
		i++;
	}
}

static int
cmd_parse_expand_alias(struct cmd_parse_command *cmd,
    struct cmd_parse_input *pi, struct cmd_parse_result *pr)
{
	struct cmd_parse_argument	*first;
	struct cmd_parse_commands	*cmds;
	struct cmd_parse_command	*last;
	char				*alias, *name, *cause;

	if (pi->flags & CMD_PARSE_NOALIAS)
		return (0);
	memset(pr, 0, sizeof *pr);

	first = TAILQ_FIRST(&cmd->arguments);
	if (first == NULL || first->type != CMD_PARSE_STRING) {
		pr->status = CMD_PARSE_SUCCESS;
		pr->cmdlist = cmd_list_new();
		return (1);
	}
	name = first->string;

	alias = cmd_get_alias(name);
	if (alias == NULL)
		return (0);
	log_debug("%s: %u alias %s = %s", __func__, pi->line, name, alias);

	cmds = cmd_parse_do_buffer(alias, strlen(alias), pi, &cause);
	free(alias);
	if (cmds == NULL) {
		pr->status = CMD_PARSE_ERROR;
		pr->error = cause;
		return (1);
	}

	last = TAILQ_LAST(cmds, cmd_parse_commands);
	if (last == NULL) {
		pr->status = CMD_PARSE_SUCCESS;
		pr->cmdlist = cmd_list_new();
		return (1);
	}

	TAILQ_REMOVE(&cmd->arguments, first, entry);
	cmd_parse_free_argument(first);

	TAILQ_CONCAT(&last->arguments, &cmd->arguments, entry);
	cmd_parse_log_commands(cmds, __func__);

	pi->flags |= CMD_PARSE_NOALIAS;
	cmd_parse_build_commands(cmds, pi, pr);
	pi->flags &= ~CMD_PARSE_NOALIAS;
	return (1);
}

static void
cmd_parse_build_command(struct cmd_parse_command *cmd,
    struct cmd_parse_input *pi, struct cmd_parse_result *pr)
{
	struct cmd_parse_argument	*arg;
	struct cmd			*add;
	char				*cause;
	struct args_value		*values = NULL;
	u_int				 count = 0, idx;

	memset(pr, 0, sizeof *pr);

	if (cmd_parse_expand_alias(cmd, pi, pr))
		return;

	TAILQ_FOREACH(arg, &cmd->arguments, entry) {
		values = xrecallocarray(values, count, count + 1,
		    sizeof *values);
		switch (arg->type) {
		case CMD_PARSE_STRING:
			values[count].type = ARGS_STRING;
			values[count].string = xstrdup(arg->string);
			break;
		case CMD_PARSE_COMMANDS:
			cmd_parse_build_commands(arg->commands, pi, pr);
			if (pr->status != CMD_PARSE_SUCCESS)
				goto out;
			values[count].type = ARGS_COMMANDS;
			values[count].cmdlist = pr->cmdlist;
			break;
		case CMD_PARSE_PARSED_COMMANDS:
			values[count].type = ARGS_COMMANDS;
			values[count].cmdlist = arg->cmdlist;
			values[count].cmdlist->references++;
			break;
		}
		count++;
	}

	add = cmd_parse(values, count, pi->file, pi->line, pi->flags, &cause);
	if (add == NULL) {
		pr->status = CMD_PARSE_ERROR;
		pr->error = cmd_parse_get_error(pi->file, pi->line, cause);
		free(cause);
		goto out;
	}
	pr->status = CMD_PARSE_SUCCESS;
	pr->cmdlist = cmd_list_new();
	cmd_list_append(pr->cmdlist, add);

out:
	for (idx = 0; idx < count; idx++)
		args_free_value(&values[idx]);
	free(values);
}

static void
cmd_parse_build_commands(struct cmd_parse_commands *cmds,
    struct cmd_parse_input *pi, struct cmd_parse_result *pr)
{
	struct cmd_parse_command	*cmd;
	u_int				 line = UINT_MAX;
	struct cmd_list			*current = NULL, *result;
	char				*s;

	memset(pr, 0, sizeof *pr);

	/* Check for an empty list. */
	if (TAILQ_EMPTY(cmds)) {
		pr->status = CMD_PARSE_SUCCESS;
		pr->cmdlist = cmd_list_new();
		return;
	}
	cmd_parse_log_commands(cmds, __func__);

	/*
	 * Parse each command into a command list. Create a new command list
	 * for each line (unless the flag is set) so they get a new group (so
	 * the queue knows which ones to remove if a command fails when
	 * executed).
	 */
	result = cmd_list_new();
	TAILQ_FOREACH(cmd, cmds, entry) {
		if (((~pi->flags & CMD_PARSE_ONEGROUP) && cmd->line != line)) {
			if (current != NULL) {
				cmd_parse_print_commands(pi, current);
				cmd_list_move(result, current);
				cmd_list_free(current);
			}
			current = cmd_list_new();
		}
		if (current == NULL)
			current = cmd_list_new();
		line = pi->line = cmd->line;

		cmd_parse_build_command(cmd, pi, pr);
		if (pr->status != CMD_PARSE_SUCCESS) {
			cmd_list_free(result);
			cmd_list_free(current);
			return;
		}
		cmd_list_append_all(current, pr->cmdlist);
		cmd_list_free(pr->cmdlist);
	}
	if (current != NULL) {
		cmd_parse_print_commands(pi, current);
		cmd_list_move(result, current);
		cmd_list_free(current);
	}

	s = cmd_list_print(result, 0);
	log_debug("%s: %s", __func__, s);
	free(s);

	pr->status = CMD_PARSE_SUCCESS;
	pr->cmdlist = result;
}

struct cmd_parse_result *
cmd_parse_from_file(FILE *f, struct cmd_parse_input *pi)
{
	static struct cmd_parse_result	 pr;
	struct cmd_parse_input		 input;
	struct cmd_parse_commands	*cmds;
	char				*cause;

	if (pi == NULL) {
		memset(&input, 0, sizeof input);
		pi = &input;
	}
	memset(&pr, 0, sizeof pr);

	cmds = cmd_parse_do_file(f, pi, &cause);
	if (cmds == NULL) {
		pr.status = CMD_PARSE_ERROR;
		pr.error = cause;
		return (&pr);
	}
	cmd_parse_build_commands(cmds, pi, &pr);
	cmd_parse_free_commands(cmds);
	return (&pr);

}

struct cmd_parse_result *
cmd_parse_from_string(const char *s, struct cmd_parse_input *pi)
{
	struct cmd_parse_input	input;

	if (pi == NULL) {
		memset(&input, 0, sizeof input);
		pi = &input;
	}

	/*
	 * When parsing a string, put commands in one group even if there are
	 * multiple lines. This means { a \n b } is identical to "a ; b" when
	 * given as an argument to another command.
	 */
	pi->flags |= CMD_PARSE_ONEGROUP;
	return (cmd_parse_from_buffer(s, strlen(s), pi));
}

enum cmd_parse_status
cmd_parse_and_insert(const char *s, struct cmd_parse_input *pi,
    struct cmdq_item *after, struct cmdq_state *state, char **error)
{
	struct cmd_parse_result	*pr;
	struct cmdq_item	*item;

	pr = cmd_parse_from_string(s, pi);
	switch (pr->status) {
	case CMD_PARSE_ERROR:
		if (error != NULL)
			*error = pr->error;
		else
			free(pr->error);
		break;
	case CMD_PARSE_SUCCESS:
		item = cmdq_get_command(pr->cmdlist, state);
		cmdq_insert_after(after, item);
		cmd_list_free(pr->cmdlist);
		break;
	}
	return (pr->status);
}

enum cmd_parse_status
cmd_parse_and_append(const char *s, struct cmd_parse_input *pi,
    struct client *c, struct cmdq_state *state, char **error)
{
	struct cmd_parse_result	*pr;
	struct cmdq_item	*item;

	pr = cmd_parse_from_string(s, pi);
	switch (pr->status) {
	case CMD_PARSE_ERROR:
		if (error != NULL)
			*error = pr->error;
		else
			free(pr->error);
		break;
	case CMD_PARSE_SUCCESS:
		item = cmdq_get_command(pr->cmdlist, state);
		cmdq_append(c, item);
		cmd_list_free(pr->cmdlist);
		break;
	}
	return (pr->status);
}

struct cmd_parse_result *
cmd_parse_from_buffer(const void *buf, size_t len, struct cmd_parse_input *pi)
{
	static struct cmd_parse_result	 pr;
	struct cmd_parse_input		 input;
	struct cmd_parse_commands	*cmds;
	char				*cause;

	if (pi == NULL) {
		memset(&input, 0, sizeof input);
		pi = &input;
	}
	memset(&pr, 0, sizeof pr);

	if (len == 0) {
		pr.status = CMD_PARSE_SUCCESS;
		pr.cmdlist = cmd_list_new();
		return (&pr);
	}

	cmds = cmd_parse_do_buffer(buf, len, pi, &cause);
	if (cmds == NULL) {
		pr.status = CMD_PARSE_ERROR;
		pr.error = cause;
		return (&pr);
	}
	cmd_parse_build_commands(cmds, pi, &pr);
	cmd_parse_free_commands(cmds);
	return (&pr);
}

struct cmd_parse_result *
cmd_parse_from_arguments(struct args_value *values, u_int count,
    struct cmd_parse_input *pi)
{
	static struct cmd_parse_result	 pr;
	struct cmd_parse_input		 input;
	struct cmd_parse_commands	*cmds;
	struct cmd_parse_command	*cmd;
	struct cmd_parse_argument	*arg;
	u_int				 i;
	char				*copy;
	size_t				 size;
	int				 end;

	/*
	 * The commands are already split up into arguments, so just separate
	 * into a set of commands by ';'.
	 */

	if (pi == NULL) {
		memset(&input, 0, sizeof input);
		pi = &input;
	}
	memset(&pr, 0, sizeof pr);

	cmds = cmd_parse_new_commands();

	cmd = xcalloc(1, sizeof *cmd);
	cmd->line = pi->line;
	TAILQ_INIT(&cmd->arguments);

	for (i = 0; i < count; i++) {
		end = 0;
		if (values[i].type == ARGS_STRING) {
			copy = xstrdup(values[i].string);
			size = strlen(copy);
			if (size != 0 && copy[size - 1] == ';') {
				copy[--size] = '\0';
				if (size > 0 && copy[size - 1] == '\\')
					copy[size - 1] = ';';
				else
					end = 1;
			}
			if (!end || size != 0) {
				arg = xcalloc(1, sizeof *arg);
				arg->type = CMD_PARSE_STRING;
				arg->string = copy;
				TAILQ_INSERT_TAIL(&cmd->arguments, arg, entry);
			} else
				free(copy);
		} else if (values[i].type == ARGS_COMMANDS) {
			arg = xcalloc(1, sizeof *arg);
			arg->type = CMD_PARSE_PARSED_COMMANDS;
			arg->cmdlist = values[i].cmdlist;
			arg->cmdlist->references++;
			TAILQ_INSERT_TAIL(&cmd->arguments, arg, entry);
		} else
			fatalx("unknown argument type");
		if (end) {
			TAILQ_INSERT_TAIL(cmds, cmd, entry);
			cmd = xcalloc(1, sizeof *cmd);
			cmd->line = pi->line;
			TAILQ_INIT(&cmd->arguments);
		}
	}
	if (!TAILQ_EMPTY(&cmd->arguments))
		TAILQ_INSERT_TAIL(cmds, cmd, entry);
	else
		free(cmd);

	cmd_parse_build_commands(cmds, pi, &pr);
	cmd_parse_free_commands(cmds);
	return (&pr);
}

static void printflike(1, 2)
yyerror(const char *fmt, ...)
{
	struct cmd_parse_state	*ps = &parse_state;
	struct cmd_parse_input	*pi = ps->input;
	va_list			 ap;
	char			*error;

	if (ps->error != NULL)
		return;

	va_start(ap, fmt);
	xvasprintf(&error, fmt, ap);
	va_end(ap);

	ps->error = cmd_parse_get_error(pi->file, pi->line, error);
	free(error);
}

static int
yylex_is_var(char ch, int first)
{
	if (ch == '=')
		return (0);
	if (first && isdigit((u_char)ch))
		return (0);
	return (isalnum((u_char)ch) || ch == '_');
}

static void
yylex_append(char **buf, size_t *len, const char *add, size_t addlen)
{
	if (addlen > SIZE_MAX - 1 || *len > SIZE_MAX - 1 - addlen)
		fatalx("buffer is too big");
	*buf = xrealloc(*buf, (*len) + 1 + addlen);
	memcpy((*buf) + *len, add, addlen);
	(*len) += addlen;
}

static void
yylex_append1(char **buf, size_t *len, char add)
{
	yylex_append(buf, len, &add, 1);
}

static int
yylex_getc1(void)
{
	struct cmd_parse_state	*ps = &parse_state;
	int			 ch;

	if (ps->f != NULL)
		ch = getc(ps->f);
	else {
		if (ps->off == ps->len)
			ch = EOF;
		else
			ch = ps->buf[ps->off++];
	}
	return (ch);
}

static void
yylex_ungetc(int ch)
{
	struct cmd_parse_state	*ps = &parse_state;

	if (ps->f != NULL)
		ungetc(ch, ps->f);
	else if (ps->off > 0 && ch != EOF)
		ps->off--;
}

static int
yylex_getc(void)
{
	struct cmd_parse_state	*ps = &parse_state;
	int			 ch;

	if (ps->escapes != 0) {
		ps->escapes--;
		return ('\\');
	}
	for (;;) {
		ch = yylex_getc1();
		if (ch == '\\') {
			ps->escapes++;
			continue;
		}
		if (ch == '\n' && (ps->escapes % 2) == 1) {
			ps->input->line++;
			ps->escapes--;
			continue;
		}

		if (ps->escapes != 0) {
			yylex_ungetc(ch);
			ps->escapes--;
			return ('\\');
		}
		return (ch);
	}
}

static char *
yylex_get_word(int ch)
{
	char	*buf;
	size_t	 len;

	len = 0;
	buf = xmalloc(1);

	do
		yylex_append1(&buf, &len, ch);
	while ((ch = yylex_getc()) != EOF && strchr(" \t\n", ch) == NULL);
	yylex_ungetc(ch);

	buf[len] = '\0';
	log_debug("%s: %s", __func__, buf);
	return (buf);
}

static int
yylex(void)
{
	struct cmd_parse_state	*ps = &parse_state;
	char			*token, *cp;
	int			 ch, next, condition;

	if (ps->eol)
		ps->input->line++;
	ps->eol = 0;

	condition = ps->condition;
	ps->condition = 0;

	for (;;) {
		ch = yylex_getc();

		if (ch == EOF) {
			/*
			 * Ensure every file or string is terminated by a
			 * newline. This keeps the parser simpler and avoids
			 * having to add a newline to each string.
			 */
			if (ps->eof)
				break;
			ps->eof = 1;
			return ('\n');
		}

		if (ch == ' ' || ch == '\t') {
			/*
			 * Ignore whitespace.
			 */
			continue;
		}

		if (ch == '\r') {
			/*
			 * Treat \r\n as \n.
			 */
			ch = yylex_getc();
			if (ch != '\n') {
				yylex_ungetc(ch);
				ch = '\r';
			}
		}
		if (ch == '\n') {
			/*
			 * End of line. Update the line number.
			 */
			ps->eol = 1;
			return ('\n');
		}

		if (ch == ';' || ch == '{' || ch == '}') {
			/*
			 * A semicolon or { or } is itself.
			 */
			return (ch);
		}

		if (ch == '#') {
			/*
			 * #{ after a condition opens a format; anything else
			 * is a comment, ignore up to the end of the line.
			 */
			next = yylex_getc();
			if (condition && next == '{') {
				yylval.token = yylex_format();
				if (yylval.token == NULL)
					return (ERROR);
				return (FORMAT);
			}
			while (next != '\n' && next != EOF)
				next = yylex_getc();
			if (next == '\n') {
				ps->input->line++;
				return ('\n');
			}
			continue;
		}

		if (ch == '%') {
			/*
			 * % is a condition unless it is all % or all numbers,
			 * then it is a token.
			 */
			yylval.token = yylex_get_word('%');
			for (cp = yylval.token; *cp != '\0'; cp++) {
				if (*cp != '%' && !isdigit((u_char)*cp))
					break;
			}
			if (*cp == '\0')
				return (TOKEN);
			ps->condition = 1;
			if (strcmp(yylval.token, "%hidden") == 0) {
				free(yylval.token);
				return (HIDDEN);
			}
			if (strcmp(yylval.token, "%if") == 0) {
				free(yylval.token);
				return (IF);
			}
			if (strcmp(yylval.token, "%else") == 0) {
				free(yylval.token);
				return (ELSE);
			}
			if (strcmp(yylval.token, "%elif") == 0) {
				free(yylval.token);
				return (ELIF);
			}
			if (strcmp(yylval.token, "%endif") == 0) {
				free(yylval.token);
				return (ENDIF);
			}
			free(yylval.token);
			return (ERROR);
		}

		/*
		 * Otherwise this is a token.
		 */
		token = yylex_token(ch);
		if (token == NULL)
			return (ERROR);
		yylval.token = token;

		if (strchr(token, '=') != NULL && yylex_is_var(*token, 1)) {
			for (cp = token + 1; *cp != '='; cp++) {
				if (!yylex_is_var(*cp, 0))
					break;
			}
			if (*cp == '=')
				return (EQUALS);
		}
		return (TOKEN);
	}
	return (0);
}

static char *
yylex_format(void)
{
	char	*buf;
	size_t	 len;
	int	 ch, brackets = 1;

	len = 0;
	buf = xmalloc(1);

	yylex_append(&buf, &len, "#{", 2);
	for (;;) {
		if ((ch = yylex_getc()) == EOF || ch == '\n')
			goto error;
		if (ch == '#') {
			if ((ch = yylex_getc()) == EOF || ch == '\n')
				goto error;
			if (ch == '{')
				brackets++;
			yylex_append1(&buf, &len, '#');
		} else if (ch == '}') {
			if (brackets != 0 && --brackets == 0) {
				yylex_append1(&buf, &len, ch);
				break;
			}
		}
		yylex_append1(&buf, &len, ch);
	}
	if (brackets != 0)
		goto error;

	buf[len] = '\0';
	log_debug("%s: %s", __func__, buf);
	return (buf);

error:
	free(buf);
	return (NULL);
}

static int
yylex_token_escape(char **buf, size_t *len)
{
	int	 ch, type, o2, o3, mlen;
	u_int	 size, i, tmp;
	char	 s[9], m[MB_LEN_MAX];

	ch = yylex_getc();

	if (ch >= '4' && ch <= '7') {
		yyerror("invalid octal escape");
		return (0);
	}
	if (ch >= '0' && ch <= '3') {
		o2 = yylex_getc();
		if (o2 >= '0' && o2 <= '7') {
			o3 = yylex_getc();
			if (o3 >= '0' && o3 <= '7') {
				ch = 64 * (ch - '0') +
				      8 * (o2 - '0') +
					  (o3 - '0');
				yylex_append1(buf, len, ch);
				return (1);
			}
		}
		yyerror("invalid octal escape");
		return (0);
	}

	switch (ch) {
	case EOF:
		return (0);
	case 'a':
		ch = '\a';
		break;
	case 'b':
		ch = '\b';
		break;
	case 'e':
		ch = '\033';
		break;
	case 'f':
		ch = '\f';
		break;
	case 's':
		ch = ' ';
		break;
	case 'v':
		ch = '\v';
		break;
	case 'r':
		ch = '\r';
		break;
	case 'n':
		ch = '\n';
		break;
	case 't':
		ch = '\t';
		break;
	case 'u':
		type = 'u';
		size = 4;
		goto unicode;
	case 'U':
		type = 'U';
		size = 8;
		goto unicode;
	}

	yylex_append1(buf, len, ch);
	return (1);

unicode:
	for (i = 0; i < size; i++) {
		ch = yylex_getc();
		if (ch == EOF || ch == '\n')
			return (0);
		if (!isxdigit((u_char)ch)) {
			yyerror("invalid \\%c argument", type);
			return (0);
		}
		s[i] = ch;
	}
	s[i] = '\0';

	if ((size == 4 && sscanf(s, "%4x", &tmp) != 1) ||
	    (size == 8 && sscanf(s, "%8x", &tmp) != 1)) {
		yyerror("invalid \\%c argument", type);
		return (0);
	}
	mlen = wctomb(m, tmp);
	if (mlen <= 0 || mlen > (int)sizeof m) {
		yyerror("invalid \\%c argument", type);
		return (0);
	}
	yylex_append(buf, len, m, mlen);
	return (1);
}

static int
yylex_token_variable(char **buf, size_t *len)
{
	struct environ_entry	*envent;
	int			 ch, brackets = 0;
	char			 name[1024];
	size_t			 namelen = 0;
	const char		*value;

	ch = yylex_getc();
	if (ch == EOF)
		return (0);
	if (ch == '{')
		brackets = 1;
	else {
		if (!yylex_is_var(ch, 1)) {
			yylex_append1(buf, len, '$');
			yylex_ungetc(ch);
			return (1);
		}
		name[namelen++] = ch;
	}

	for (;;) {
		ch = yylex_getc();
		if (brackets && ch == '}')
			break;
		if (ch == EOF || !yylex_is_var(ch, 0)) {
			if (!brackets) {
				yylex_ungetc(ch);
				break;
			}
			yyerror("invalid environment variable");
			return (0);
		}
		if (namelen == (sizeof name) - 2) {
			yyerror("environment variable is too long");
			return (0);
		}
		name[namelen++] = ch;
	}
	name[namelen] = '\0';

	envent = environ_find(global_environ, name);
	if (envent != NULL && envent->value != NULL) {
		value = envent->value;
		log_debug("%s: %s -> %s", __func__, name, value);
		yylex_append(buf, len, value, strlen(value));
	}
	return (1);
}

static int
yylex_token_tilde(char **buf, size_t *len)
{
	struct environ_entry	*envent;
	int			 ch;
	char			 name[1024];
	size_t			 namelen = 0;
	struct passwd		*pw;
	const char		*home = NULL;

	for (;;) {
		ch = yylex_getc();
		if (ch == EOF || strchr("/ \t\n\"'", ch) != NULL) {
			yylex_ungetc(ch);
			break;
		}
		if (namelen == (sizeof name) - 2) {
			yyerror("user name is too long");
			return (0);
		}
		name[namelen++] = ch;
	}
	name[namelen] = '\0';

	if (*name == '\0') {
		envent = environ_find(global_environ, "HOME");
		if (envent != NULL && *envent->value != '\0')
			home = envent->value;
		else if ((pw = getpwuid(getuid())) != NULL)
			home = pw->pw_dir;
	} else {
		if ((pw = getpwnam(name)) != NULL)
			home = pw->pw_dir;
	}
	if (home == NULL)
		return (0);

	log_debug("%s: ~%s -> %s", __func__, name, home);
	yylex_append(buf, len, home, strlen(home));
	return (1);
}

static char *
yylex_token(int ch)
{
	struct cmd_parse_state	*ps = &parse_state;
	char			*buf;
	size_t			 len;
	enum { START,
	       NONE,
	       DOUBLE_QUOTES,
	       SINGLE_QUOTES }	 state = NONE, last = START;

	len = 0;
	buf = xmalloc(1);

	for (;;) {
		/* EOF or \n are always the end of the token. */
		if (ch == EOF) {
			log_debug("%s: end at EOF", __func__);
			break;
		}
		if (state == NONE && ch == '\r') {
			ch = yylex_getc();
			if (ch != '\n') {
				yylex_ungetc(ch);
				ch = '\r';
			}
		}
		if (ch == '\n') {
			if (state == NONE) {
				log_debug("%s: end at EOL", __func__);
				break;
			}
			ps->input->line++;
		}

		/* Whitespace or ; or } ends a token unless inside quotes. */
		if (state == NONE && (ch == ' ' || ch == '\t')) {
			log_debug("%s: end at WS", __func__);
			break;
		}
		if (state == NONE && (ch == ';' || ch == '}')) {
			log_debug("%s: end at %c", __func__, ch);
			break;
		}

		/*
		 * Spaces and comments inside quotes after \n are removed but
		 * the \n is left.
		 */
		if (ch == '\n' && state != NONE) {
			yylex_append1(&buf, &len, '\n');
			while ((ch = yylex_getc()) == ' ' || ch == '\t')
				/* nothing */;
			if (ch != '#')
				continue;
			ch = yylex_getc();
			if (strchr(",#{}:", ch) != NULL) {
				yylex_ungetc(ch);
				ch = '#';
			} else {
				while ((ch = yylex_getc()) != '\n' && ch != EOF)
					/* nothing */;
			}
			continue;
		}

		/* \ ~ and $ are expanded except in single quotes. */
		if (ch == '\\' && state != SINGLE_QUOTES) {
			if (!yylex_token_escape(&buf, &len))
				goto error;
			goto skip;
		}
		if (ch == '~' && last != state && state != SINGLE_QUOTES) {
			if (!yylex_token_tilde(&buf, &len))
				goto error;
			goto skip;
		}
		if (ch == '$' && state != SINGLE_QUOTES) {
			if (!yylex_token_variable(&buf, &len))
				goto error;
			goto skip;
		}
		if (ch == '}' && state == NONE)
			goto error;  /* unmatched (matched ones were handled) */

		/* ' and " starts or end quotes (and is consumed). */
		if (ch == '\'') {
			if (state == NONE) {
				state = SINGLE_QUOTES;
				goto next;
			}
			if (state == SINGLE_QUOTES) {
				state = NONE;
				goto next;
			}
		}
		if (ch == '"') {
			if (state == NONE) {
				state = DOUBLE_QUOTES;
				goto next;
			}
			if (state == DOUBLE_QUOTES) {
				state = NONE;
				goto next;
			}
		}

		/* Otherwise add the character to the buffer. */
		yylex_append1(&buf, &len, ch);

	skip:
		last = state;

	next:
		ch = yylex_getc();
	}
	yylex_ungetc(ch);

	buf[len] = '\0';
	log_debug("%s: %s", __func__, buf);
	return (buf);

error:
	free(buf);
	return (NULL);
}
