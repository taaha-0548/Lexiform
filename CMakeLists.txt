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
)

if (EMSCRIPTEN)
    # Build WebAssembly module for the JS frontend
    add_executable(formscript_wasm src/WasmBindings.cpp ${CORE_SOURCES})
    
    set_target_properties(formscript_wasm PROPERTIES 
        OUTPUT_NAME "formscript"
        SUFFIX ".js"
    )
    
    # Emscripten specific flags for WASM and Embind
    target_link_options(formscript_wasm PRIVATE 
        "--bind" 
        "-s" "WASM=1" 
        "-s" "MODULARIZE=1" 
        "-s" "EXPORT_ES6=1" 
        "-s" "EXPORT_NAME='createFormScriptModule'"
        "-s" "ALLOW_MEMORY_GROWTH=1"
        "-O3"
    )
else()
    # Standard CLI compiler
    add_executable(formscript src/main.cpp ${CORE_SOURCES})
    install(TARGETS formscript DESTINATION bin)
endif()
