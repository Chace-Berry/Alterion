cmake_minimum_required(VERSION 3.15)

# Core lexer library
add_library(alterion_lexer
    lexer/lexer.cpp
    lexer/token.cpp
)

target_include_directories(alterion_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Core parser library
add_library(alterion_parser
    parser/parser_enhanced.cpp
)

target_link_libraries(alterion_parser alterion_lexer)
target_include_directories(alterion_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Semantic analysis (if exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/semantic/semantic_analysis.cpp")
    add_library(alterion_semantic
        semantic/semantic_analysis.cpp
    )
    target_link_libraries(alterion_semantic alterion_parser)
    target_include_directories(alterion_semantic PUBLIC ${CMAKE_SOURCE_DIR}/include)
endif()

# Code generation (if exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/codegen/codegen.cpp")
    add_library(alterion_codegen
        codegen/codegen.cpp
    )
    target_link_libraries(alterion_codegen alterion_parser)
    target_include_directories(alterion_codegen PUBLIC ${CMAKE_SOURCE_DIR}/include)
endif()
