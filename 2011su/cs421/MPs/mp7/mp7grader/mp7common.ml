(* File: mp7common.ml *)

type position = {line_num : int; char_num : int}

exception OpenComm of position
exception CloseComm of position

type token =
  | INT of (int)
  | REAL of (float)
  | BOOL of (bool)
  | STRING of (string)
  | IDENT of (string)
  | OPCOM of ((int*int))
  | CLCOM of ((int*int))
  | PLUS
  | MINUS
  | TIMES
  | DIV
  | DPLUS
  | DMINUS
  | DTIMES
  | DDIV
  | CARAT
  | LT
  | GT
  | LEQ
  | GEQ
  | EQUALS
  | AND
  | PIPE
  | ARROW
  | DCOLON
  | SEMI
  | AT
  | NEG
  | NEQ
  | NIL
  | LET
  | MOD
  | NOT
  | REC
  | IN
  | IF
  | THEN
  | ELSE
  | FUN
  | VAL
  | WITH
  | RAISE
  | ORELSE
  | OP
  | LOCAL
  | HANDLE
  | FN
  | END
  | ANDALSO
  | LBRAC
  | RBRAC
  | LPAREN
  | RPAREN
  | COMMA
  | UNDERSCORE
  | UNIT
  | ERROR
  | EOF
  | DBLSEMI
  | EXP
  | OR

