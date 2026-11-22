# Regenerates the lexer and parser in generated/ from BasicLexer.g4 and
# BasicParser.g4, then records the grammar's hash in generated/grammar.sha256.
#
#   cmake -DJAVA=<java> -DJAR=<antlr jar> -DSOURCE_DIR=<repo root> -P generate_parser.cmake
#
# ANTLR runs from the repo root with relative paths so the "Generated from"
# comment in each file doesn't depend on where the repo is checked out.

include("${CMAKE_CURRENT_LIST_DIR}/grammar_hash.cmake")

execute_process(
  COMMAND "${JAVA}" -jar "${JAR}" -Dlanguage=Cpp -package wwivbasic -visitor -no-listener
          -Werror -o generated BasicLexer.g4 BasicParser.g4
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "ANTLR failed to generate the parser (exit code ${result})")
endif()

wwivbasic_grammar_hash("${SOURCE_DIR}" "${JAR}" hash)
file(WRITE "${SOURCE_DIR}/generated/grammar.sha256" "${hash}\n")
