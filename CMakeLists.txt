cmake_minimum_required(VERSION 3.8)
project(antlrsample)

set(CMAKE_CXX_STANDARD 17)

find_package(antlr4-runtime REQUIRED)
find_package(antlr4-generator REQUIRED)
find_package(fmt CONFIG REQUIRED)

 set(ANTLR4_JAR_LOCATION ${PROJECT_SOURCE_DIR}/antlr/antlr-4.12.0-complete.jar)

 # required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# generate lexer
 antlr4_generate(
   wwivbasic_lexer
   ${CMAKE_CURRENT_SOURCE_DIR}/BasicLexer.g4
   LEXER
   FALSE
   FALSE
   "wwivbasic"
   )

 # generate parser
 antlr4_generate(
   wwivbasic_parser
   ${CMAKE_CURRENT_SOURCE_DIR}/BasicParser.g4
   PARSER
   FALSE
   TRUE
   "wwivbasic"
   "${ANTLR4_TOKEN_FILES_wwivbasic_lexer}"
   "${ANTLR4_TOKEN_DIRECTORY_wwivbasic_lexer}"
   )
   
add_executable(antlrsample 
               "src/main.cpp" 
               "src/execution_context.cpp"
               "src/executor.cpp"
               "src/function_def_visitor.cpp"
               ${ANTLR4_SRC_FILES_wwivbasic_lexer} 
               ${ANTLR4_SRC_FILES_wwivbasic_parser}
               )

target_link_libraries(antlrsample antlr4_shared fmt::fmt-header-only)

target_include_directories(antlrsample PRIVATE ${ANTLR4_INCLUDE_DIR} 
                           ${ANTLR4_INCLUDE_DIR_wwivbasic_lexer} 
                           ${ANTLR4_INCLUDE_DIR_wwivbasic_parser})