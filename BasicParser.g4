parser grammar BasicParser;

options {
	tokenVocab = BasicLexer;
}

// Follows directly after the standard #includes in h + cpp files.
@parser::postinclude {
/* parser postinclude section */
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
}

// Actual grammar start.
main
  : (
    statement 
    | procedureDefinition
    | importModule
    | moduleDefinition
   )+ EOF
;

procedureCall
  : procedureName LPAREN (parameterList)? RPAREN
  ;

parameterList
  : expr (COMMA expr)*
;

procedureDefinition
  : DEF procedureName LPAREN (parameterDefinitionList)? RPAREN statements ENDDEF
  ;

parameterDefinitionList
  : param = id (COMMA param = id)*
;

importModule
  : IMPORT AT ID  NEWLINE
  | IMPORT STRING NEWLINE
;

moduleDefinition
  : MODULE STRING NEWLINE
;

statements
  : statement*
;

statement
    : assignmentStatement   
    | ifStatement   
    | forStatement
    | procedureCall NEWLINE
    | returnStatement
    | emptyStatement        
;

emptyStatement : NEWLINE;


expr: expr relationaloperator expr      # Relation
    | expr multiplicativeoperator expr  # MulDiv
    | expr additiveoperator expr        # AddSub
    | procedureCall                     # ProcCall
    | LPAREN expr RPAREN                # Parens
    | identifier = id                   # Ident
    | INT                               # Int
    | STRING                            # String
;

additiveoperator
   : PLUS
   | MINUS
   | OR
;

multiplicativeoperator
   : STAR
   | SLASH
   | MOD
   | AND
;

relationaloperator
   : EQ
   | NE
   | LT
   | LE
   | GE
   | GT
   ;

// Statements

assignmentStatement 
  : <assoc = right> variable EQ expr NEWLINE
  ;

// conditional
ifStatement
  : ifThenStatement
  | ifThenElseStatement
  | ifThenElseIfElseStatement
;

ifThenStatement
  : IF expr THEN NEWLINE? statements ENDIF NEWLINE
;

ifThenElseStatement
  : IF expr THEN NEWLINE? statements ELSE statements NEWLINE? ENDIF NEWLINE
;

ifThenElseIfElseStatement
  : IF expr THEN NEWLINE? statements (ELSEIF expr THEN NEWLINE? statements)* ELSE statements ENDIF NEWLINE
;

forStatement
  : FOR ID EQ expr TO expr (STEP INT)? NEWLINE? statements NEXT NEWLINE
;

returnStatement
  : RETURN expr NEWLINE
;

id: ID;

variable :   ID;
procedureName : ID;

