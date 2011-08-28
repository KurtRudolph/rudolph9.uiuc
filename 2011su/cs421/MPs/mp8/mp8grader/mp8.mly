/* Use the expression datatype defined in mp8common.ml: */
%{
    open Mp8common

(* add any extra code here *)


%}

/* Define the tokens of the language: */
%token <int> INT
%token <float> REAL
%token <bool> BOOL
%token <string> STRING IDENT
%token <(int*int)> OPCOM CLCOM
%token NEG PLUS MINUS TIMES DIV DPLUS DMINUS DTIMES DDIV CARAT LT GT LEQ GEQ
       EQUALS NEQ PIPE ARROW SEMI DCOLON AT NIL LET LOCAL VAL REC AND END IN
       IF THEN ELSE FUN FN OP MOD RAISE HANDLE WITH NOT ANDALSO ORELSE
       HD TL FST SND
       LBRAC RBRAC LPAREN RPAREN COMMA UNDERSCORE
       UNIT ERROR EOF

/* Define the "goal" nonterminal of the grammar: */
%start main
%type <Mp8common.dec> main

%%

main:
    expression SEMI                             { Val(Some "it", $1) }
  | dec SEMI                                    { $1 }

expression:
    atomic_expression { $1 } /* You will want to change this line. It is only here to enable compilation */
    /* You need work here */

atomic_expression:
    IDENT             { VarExp $1 }
    /* You need work here */

dec:
    atomic_dec                                  { $1 }
  | dec atomic_dec                              { Seq($1, $2) }

atomic_dec:
    VAL IDENT EQUALS expression { Val (Some $2, $4) } /* You will probably want to change this line */
   /* You need work here */


/* You need work here */
