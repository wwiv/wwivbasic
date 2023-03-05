lexer grammar BasicLexer;

// These are all supported lexer sections:

// Lexer file header. Appears at the top of h + cpp files. Use e.g. for copyrights.
@lexer::header {/* lexer header section */}

// Appears before any #include in h + cpp files.
@lexer::preinclude {/* lexer precinclude section */}

// Follows directly after the standard #includes in h + cpp files.
@lexer::postinclude {
/* lexer postinclude section */
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
}

// Directly preceds the lexer class declaration in the h file (e.g. for additional types etc.).
@lexer::context {/* lexer context section */}

// Appears in the public part of the lexer in the h file.
@lexer::members {/* public lexer declarations section */
}

// Appears in the private part of the lexer in the h file.
@lexer::declarations {/* private lexer declarations/members section */}

// Appears in line with the other class member definitions in the cpp file.
@lexer::definitions {/* lexer definitions section */}
options { caseInsensitive=true; }

channels { CommentsChannel, DirectiveChannel }

tokens {
	DUMMY
}

INT: Digit+;
Digit: [0-9];

TRUE: 'TRUE';
FALSE: 'FALSE';

// BinaryOperators

LT: '<';
LE: '<=';
GT:  '>';
GE:  '>=';
EQ: '=';
NE: '<>';
AND: 'AND';
OR: 'OR';
NOT: 'NOT';
LET: 'LET';
MOD: 'MOD';

// General Tokens
COLON: ':';
SEMICOLON: ';';
PLUS: '+';
MINUS: '-';
STAR: '*';
SLASH: '/';
LPAREN: '(';
RPAREN: ')';
COMMA: ',';
DOT: '.';
AT: '@';
 

// KEYWORDS

// methods
DEF : 'DEF';
ENDDEF : 'ENDDEF';
RETURN : 'RETURN';

// control flow
IF: 'IF';
THEN: 'THEN';
ELSE: 'ELSE';
ELSEIF : 'ELSEIF';
ENDIF: 'ENDIF';

FOR: 'FOR';
NEXT: 'NEXT';
STEP: 'STEP';
TO: 'TO';

// modules
MODULE: 'MODULE';
IMPORT: 'IMPORT';

STRING options { caseInsensitive=false; } : '"' .*? '"';
ID: LETTER (LETTER | '.' | '0'..'9')*;
fragment LETTER : [A-Z];

Comment : '\'' ~[\r\n]* '\r'? '\n' -> channel(CommentsChannel);
WS: [ \t]+ -> channel(99);
NEWLINE: WS? ('\r'? '\n') WS?;
