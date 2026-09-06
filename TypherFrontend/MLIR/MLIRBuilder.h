#ifndef MLIR_BUILDER_H
#define MLIR_BUILDER_H

#include "mlir/Dialect/Func/Extensions/AllExtensions.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/Transforms/InlinerInterfaceImpl.h"

#include "mlir/Dialect/Affine/Transforms/Passes.h"
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h"
#include "mlir/ExecutionEngine/ExecutionEngine.h"
#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "mlir/Transforms/Passes.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"

#include "Dialect/TypherDialect.h"
#include "Function.h"
#include "Visitor.h"
#include "Expressions/Operator.h"
#include "Expressions/CallExpression.h"
#include "Expressions/MemoryOperation.h"
#include "Expressions/InitializerList.h"
#include "statements/ReturnStatement.h"
#include "statements/IfStatement.h"
#include "statements/WhileStatement.h"
#include "statements/ForStatement.h"
#include "Literals/IntegerLiteral.h"
#include "Literals/StringLiteral.h"
#include "statements/ExpressionStatement.h"
#include <fstream>

namespace MLIR {

	class Generator;

	class Builder {
		friend class Generator;
	public:
		Builder(Generator* gen);

		~Builder() = default; 

		void Build(SlabVector<AST::Statement*>& ASTTree);	

		mlir::MLIRContext& Context()
		{
			return *context.get();
		}

		mlir::ModuleOp& Module()
		{
			return theModule;
		}

	private:
		//mlir::OwningOpRef<mlir::ModuleOp> GenTree();
		//mlir::Value HandleRefAndAdr(AST::Operator* node);
		Generator* gen_;
		mlir::Location loc(const Location &loc)
		{
			return mlir::FileLineColLoc::get(op->getStringAttr(loc.file), loc.line, loc.col);
		}

		llvm::LogicalResult declare(llvm::StringRef var, mlir::Value value)
		{
			auto persistentName = mlir::StringAttr::get(context.get(), var).getValue();
			if (symbolTable.count(persistentName))
				return mlir::failure();
			
			symbolTable.insert(persistentName, value);
			return mlir::success();
		}	

		mlir::ModuleOp theModule;
		std::shared_ptr<mlir::OpBuilder> op;
		llvm::ScopedHashTable<llvm::StringRef, mlir::Value> symbolTable;

		mlir::Value retValue;
		
		mlir::DialectRegistry registry;
		std::shared_ptr<mlir::MLIRContext> context;
		void GenBody(AST::Body* node, mlir::Location& location);

		mlir::Value LvalueToRvalue(mlir::Value val, mlir::Location loc);
		mlir::Value GenArrayAccess(AST::MemoryOperation* node, mlir::Type array_type);
	};
}

#endif
