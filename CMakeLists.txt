cmake_minimum_required(VERSION 3.10)
project(Lexiform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CORE_SOURCES 
    src/Lexer.cpp 
    src/Parser.cpp 
    src/SemanticAnalyzer.cpp 
    src/AST.cpp
    src/IRGenerator.cpp
    src/Optimizer.cpp
)

if (EMSCRIPTEN)
    # Build WebAssembly module for the JS frontend
    add_executable(lexiform_wasm src/WasmBindings.cpp ${CORE_SOURCES})
    
    set_target_properties(lexiform_wasm PROPERTIES 
        OUTPUT_NAME "lexiform"
        SUFFIX ".js"
    )
    
    # Emscripten specific flags for WASM and Embind
    target_link_options(lexiform_wasm PRIVATE 
        "--bind" 
        "-sWASM=1" 
        "-sMODULARIZE=1" 
        "-sEXPORT_ES6=1" 
        "-sEXPORT_NAME='createLexiformModule'"
        "-sALLOW_MEMORY_GROWTH=1"
        "-O3"
    )
else()
    # Standard CLI compiler
    add_executable(lexiform src/main.cpp ${CORE_SOURCES})
    install(TARGETS lexiform DESTINATION bin)
endif()
