# CMakeList.txt : CMake project for TypherCompiler, include source and define
# project specific logic here.
#

# Add source to this project's executable.


# MLIR SETUP --------------------

set(LLVM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/llvm-project" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

set(LIBS
    MLIRAnalysis
    MLIRFunctionInterfaces
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
    MLIRSupport              
    LLVMSupport    
    MLIRControlFlowInterfaces
    MLIRCallInterfaces       
    MLIRCastInterfaces                   
)

set(LLVM_BUILD_DIR "${LLVM_ROOT}/build") 

# 2. Point specifically to the folders containing the Config.cmake files
set(MLIR_DIR "${LLVM_BUILD_DIR}/lib/cmake/mlir")
set(LLVM_DIR "${LLVM_BUILD_DIR}/lib/cmake/llvm")

message(STATUS "MLIR_DIR: ${MLIR_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_PROJ_SRC}/llvm/cmake/modules")

# 3. Now find_package will work
find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH ${MLIR_CMAKE_DIR})
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})

include(AddLLVM)
include(TableGen)
include(AddMLIR)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${MLIR_GENERATED_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(MLIR/Dialect)

set(LLVM_LINK_COMPONENTS
  Support
)
link_directories(${LLVM_LIBRARY_DIRS})

add_executable (TypherCompiler 
  "Core/TypherCompiler.cpp" 
  "Core/CompilerImpl.cpp" 
  "PreProcess/Comments.cpp"
  "PreProcess/PreProcessor.cpp"
  "Parser/Lexer/Lexer.cpp" 
  "Parser/Lexer/Tokens/Token.cpp" 
  "Parser/Lexer/Keywords.cpp" 
  "Parser/Parser.cpp" 
  "Parser/ExpressionParser.cpp" 
  "Parser/DeclaratorParser.cpp" 
  "Parser/StatementParser.cpp" 
  "AST/ASTNode.cpp"
  "AST/Body.cpp"
  "AST/Literals/Literal.h"
  "AST/Statement.cpp"
  "AST/Expressions/VariableDeclarator.cpp"
  "AST/Expressions/MemoryOperation.cpp"
  "AST/Expressions/CallExpression.cpp"
  "AST/Expressions/Operator.cpp" 
  "AST/Expressions/InitializerList.cpp"
  "AST/statements/VariableDeclaration.cpp"
  "AST/statements/IfStatement.cpp"
  "AST/statements/WhileStatement.cpp"
  "AST/statements/ForStatement.cpp"
  "AST/statements/ReturnStatement.cpp"
  "AST/statements/ExpressionStatement.cpp"
  "Checker/Checker.cpp"
  "Checker/Type.cpp" 
  "Checker/SemanticAnalysis/SemanticAnalyzer.cpp"
  "Checker/SemanticAnalysis/TypeResolver.cpp" 
  "MLIR/MLIRGen.cpp"
  "MLIR/MLIRBuilder.cpp"
  "MLIR/MLIREmitter.cpp"
 )

target_include_directories(TypherCompiler
  PRIVATE
  ${CMAKE_SOURCE_DIR}/TypherFrontend/AST
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Parser
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Parser/Lexer
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Core
  ${CMAKE_SOURCE_DIR}/TypherFrontend/MLIR
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Checker
  ${CMAKE_SOURCE_DIR}/TypherFrontend/PreProcess
)

target_include_directories(TypherCompiler SYSTEM PRIVATE 
    ${LLVM_INCLUDE_DIRS} 
    ${MLIR_INCLUDE_DIRS}
)
# DIALECT -------

# add_mlir_dialect(TypherOps Typher)



add_dependencies(TypherCompiler TypherDialect)

# MLIR SETU END --------------------

llvm_map_components_to_libnames(llvm_libs 
    support 
    core 
    native 
    codegen 
    MC 
    target
)

mlir_target_link_libraries(TypherCompiler
  PUBLIC
  TypherDialect
   ${LIBS}
  # ... other MLIR dependencies
)

target_link_libraries(TypherCompiler PRIVATE 
    MLIRAnalysis
    MLIRBuiltinToLLVMIRTranslation
    MLIRCallInterfaces
    MLIRCastInterfaces
    MLIRExecutionEngine
    MLIRFunctionInterfaces
    MLIRIR
    MLIRLLVMCommonConversion
    MLIRLLVMToLLVMIRTranslation
    MLIRMemRefDialect
    MLIRParser
    MLIRPass
    MLIRRegisterAllDialects
    MLIRRegisterAllExtensions
    MLIRRegisterAllPasses
    MLIRSideEffectInterfaces
    MLIRTargetLLVMIRExport
    MLIRTransforms
    ${llvm_libs}
)

llvm_config(TypherCompiler support core irreader)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TypherCompiler PROPERTY CXX_STANDARD 20)
endif()

if(LINUX)
  set(CMAKE_C_COMPILER clang)
  target_compile_options(TypherCompiler PRIVATE -Wall -Wextra -Wpedantic)
endif()

# TODO: Add tests and install targets if needed.
