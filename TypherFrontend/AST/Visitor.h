#ifndef VISITOR_H
#define VISITOR_H


#define VISITOR void Visit(AST::Function* node) override; \
    void Visit(AST::Statement* node) override; \
    void Visit(AST::VariableDeclarator* node) override; \
    void Visit(AST::Expression* node) override; \
    void Visit(AST::Identifier* node) override; \
    void Visit(AST::IntegerLiteral* node) override; \
    void Visit(AST::StringLiteral* node) override; \
    void Visit(AST::VariableDeclaration* node) override; \
    void Visit(AST::Operator* node) override; \
    void Visit(AST::CallExpression* node) override; \
    void Visit(AST::MemoryOperation* node) override; \
    void Visit(AST::ReturnStatement* node) override; \
    void Visit(AST::IfStatement* node) override; \
    void Visit(AST::ExpressionStatement* node) override; \
    void Visit(AST::WhileStatement* node) override; \
    void Visit(AST::ForStatement* node) override; \
    void Visit(AST::InitializerList* node) override; \

namespace AST {
    class Function;
    class Statement;
    class VariableDeclarator;
    class VariableDeclaration;
    class Expression;
    class Identifier;
    class IntegerLiteral;
    class StringLiteral;
    class Operator;
    class CallExpression;
    class MemoryOperation;
    class ReturnStatement;
    class IfStatement;
    class ExpressionStatement;
    class WhileStatement;
    class ForStatement;
    class InitializerList;

    class NodeVisitor {
    public:
        virtual void Visit(Function* node) = 0;
        virtual void Visit(Statement* node) = 0;
        virtual void Visit(VariableDeclarator* node) = 0;
        virtual void Visit(VariableDeclaration* node) = 0;
        virtual void Visit(Expression* node) = 0;
        virtual void Visit(Identifier* node) = 0;
        virtual void Visit(IntegerLiteral* node) = 0;
        virtual void Visit(StringLiteral* node) = 0;
        virtual void Visit(Operator* node) = 0;
        virtual void Visit(CallExpression* node) = 0;
        virtual void Visit(MemoryOperation* node) = 0;
        virtual void Visit(ReturnStatement* node) = 0;
        virtual void Visit(IfStatement* node) = 0;
        virtual void Visit(ExpressionStatement* node) = 0;
        virtual void Visit(WhileStatement* node) = 0;
        virtual void Visit(ForStatement* node) = 0;
        virtual void Visit(InitializerList* node) = 0;
    };
}

#endif