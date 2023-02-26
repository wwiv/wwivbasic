parser grammar TParser;

options {
	tokenVocab = TLexer;
}

// These are all supported parser sections:

// Par ser file header. Appears at the top in all parser related files. Use e.g. for copyrights.
@parser::header {/* parser/listener/visitor header section */}

// Appears before any #include in h + cpp files.
@parser::preinclude {/* parser precinclude section */}

// Follows directly after the standard #includes in h + cpp files.
@parser::postinclude {
/* parser postinclude section */
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
}

// Directly preceeds the parser class declaration in the h file (e.g. for additional types etc.).
@parser::context {/* parser context section */}

// Appears in the private part of the parser in the h file.
// The function bodies could also appear in the definitions section, but I want to maximize
// Java compatibility, so we can also create a Java parser from this grammar.
// Still, some tweaking is necessary after the Java file generation (e.g. bool -> boolean).
@parser::members {
/* public parser declarations/members section */
bool myAction() { return true; }
bool doesItBlend() { return true; }
void cleanUp() {}
void doInit() {}
void doAfter() {}
}

// Appears in the public part of the parser in the h file.
@parser::declarations {/* private parser declarations section */}

// Appears in line with the other class member definitions in the cpp file.
@parser::definitions {/* parser definitions section */}

// Additionally there are similar sections for (base)listener and (base)visitor files.
@parser::listenerpreinclude {/* listener preinclude section */}
@parser::listenerpostinclude {/* listener postinclude section */}
@parser::listenerdeclarations {/* listener public declarations/members section */}
@parser::listenermembers {/* listener private declarations/members section */}
@parser::listenerdefinitions {/* listener definitions section */}

@parser::baselistenerpreinclude {/* base listener preinclude section */}
@parser::baselistenerpostinclude {/* base listener postinclude section */}
@parser::baselistenerdeclarations {/* base listener public declarations/members section */}
@parser::baselistenermembers {/* base listener private declarations/members section */}
@parser::baselistenerdefinitions {/* base listener definitions section */}

@parser::visitorpreinclude {/* visitor preinclude section */}
@parser::visitorpostinclude {/* visitor postinclude section */}
@parser::visitordeclarations {/* visitor public declarations/members section */}
@parser::visitormembers {/* visitor private declarations/members section */}
@parser::visitordefinitions {/* visitor definitions section */}

@parser::basevisitorpreinclude {/* base visitor preinclude section */}
@parser::basevisitorpostinclude {/* base visitor postinclude section */}
@parser::basevisitordeclarations {/* base visitor public declarations/members section */}
@parser::basevisitormembers {/* base visitor private declarations/members section */}
@parser::basevisitordefinitions {/* base visitor definitions section */}

// Actual grammar start.
main: statement+ EOF;

procedureCall
  : procedureName LPAREN (parameterList)? RPAREN
  ;

parameterList
  : expr (COMMA expr)*
;

statements
  : statement*
;

statement
    : assignmentStatement 
    | ifStatement
    | procedureCall NEWLINE
    | emptyStatement
;

emptyStatement : NEWLINE;


expr: expr multiplicativeoperator expr
    | expr additiveoperator expr
    | procedureCall
    | LPAREN expr RPAREN
    | identifier = id
    | INT
    | STRING
;

relationalExpression 
  : expr relationaloperator expr
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
  : IF relationalExpression THEN NEWLINE? statements ENDIF NEWLINE
  | IF relationalExpression THEN NEWLINE? statements (ELSEIF relationalExpression THEN NEWLINE? statements)* ELSE statements ENDIF NEWLINE
  | IF relationalExpression THEN NEWLINE? statements ELSE statements NEWLINE? ENDIF NEWLINE
;

id: ID;

variable :   ID;
procedureName : ID;
packageName : (ID DOT);
