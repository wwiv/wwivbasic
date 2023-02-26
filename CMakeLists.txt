cmake_minimum_required(VERSION 3.8)
project(antlrsample)

set(CMAKE_CXX_STANDARD 17)

find_package(antlr4-runtime REQUIRED)
find_package(antlr4-generator REQUIRED)

 set(ANTLR4_JAR_LOCATION ${PROJECT_SOURCE_DIR}/antlr/antlr-4.12.0-complete.jar)

 # required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# generate lexer
 antlr4_generate(
   antlrcpptest_lexer
   ${CMAKE_CURRENT_SOURCE_DIR}/TLexer.g4
   LEXER
   FALSE
   FALSE
   "antlrcpptest"
   )

 # generate parser
 antlr4_generate(
   antlrcpptest_parser
   ${CMAKE_CURRENT_SOURCE_DIR}/TParser.g4
   PARSER
   FALSE
   TRUE
   "antlrcpptest"
   "${ANTLR4_TOKEN_FILES_antlrcpptest_lexer}"
   "${ANTLR4_TOKEN_DIRECTORY_antlrcpptest_lexer}"
   )
   
add_executable(antlrsample 
               "src/main.cpp" 
               ${ANTLR4_SRC_FILES_antlrcpptest_lexer} 
               ${ANTLR4_SRC_FILES_antlrcpptest_parser}
               )

target_link_libraries(antlrsample antlr4_shared)
target_include_directories(antlrsample PRIVATE ${ANTLR4_INCLUDE_DIR} 
                           ${ANTLR4_INCLUDE_DIR_antlrcpptest_lexer} 
                           ${ANTLR4_INCLUDE_DIR_antlrcpptest_parser})