cmake_minimum_required(VERSION 3.8)
project(wwivbasicdemo)

set(CMAKE_CXX_STANDARD 17)

find_package(antlr4-runtime REQUIRED)
find_package(antlr4-generator REQUIRED)
find_package(fmt CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(core)

 set(ANTLR4_JAR_LOCATION ${PROJECT_SOURCE_DIR}/antlr/antlr-4.12.0-complete.jar)

 # required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# generate lexer
antlr4_generate(wwivbasic_lexer
                ${CMAKE_CURRENT_SOURCE_DIR}/BasicLexer.g4
                LEXER
                FALSE
                FALSE
                "wwivbasic"
)

# generate parser
antlr4_generate(wwivbasic_parser
                ${CMAKE_CURRENT_SOURCE_DIR}/BasicParser.g4
                PARSER
                FALSE
                TRUE
                "wwivbasic"
                "${ANTLR4_TOKEN_FILES_wwivbasic_lexer}"
                "${ANTLR4_TOKEN_DIRECTORY_wwivbasic_lexer}"
)
 
add_library(wwivbasic_interpreter
            "src/execution_context.cpp"
            "src/executor.cpp"
            "src/function_def_visitor.cpp"
            "src/utils.cpp"
            ${ANTLR4_SRC_FILES_wwivbasic_lexer} 
            ${ANTLR4_SRC_FILES_wwivbasic_parser}
)

target_include_directories(wwivbasic_interpreter PUBLIC ${ANTLR4_INCLUDE_DIR} 
                           ${ANTLR4_INCLUDE_DIR_wwivbasic_lexer} 
                           ${ANTLR4_INCLUDE_DIR_wwivbasic_parser})
target_link_libraries(wwivbasic_interpreter antlr4_shared fmt::fmt-header-only core)
target_compile_definitions(wwivbasic_interpreter PUBLIC _CRT_SECURE_NO_WARNINGS)



add_executable(basicrun
               "src/basicrun.cpp")
target_link_libraries(basicrun wwivbasic_interpreter)


add_executable(antlrdemo
               "src/antlrdemo.cpp")
target_link_libraries(antlrdemo wwivbasic_interpreter)
