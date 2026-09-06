#include "Expressions/VariableDeclarator.h"
#include "Expressions/Operator.h"
namespace AST{
	VariableDeclarator::VariableDeclarator(Expression* expr, 
										   Identifier* ident, 
										   std::vector<DeclaratorKind> modifiers)
		:ident_(ident), expression(expr), 
		 modifiers_(modifiers), Expression(AstNodeType::VARIABLE_DECLARATOR)
	{
		if (expr != nullptr) {
			expr->SetParent(this);
		}
	}

	VariableDeclarator::VariableDeclarator(Identifier* ident, std::vector<DeclaratorKind> modifiers)
		: ident_(ident), modifiers_(modifiers), Expression(AstNodeType::VARIABLE_DECLARATOR)
	{
		if (ident != nullptr) {
			ident->SetParent(this);
		}
	}

	bool VariableDeclarator::IsDefinition()
	{
		if(expression->NodeType() == AstNodeType::OPERATOR) {
			AST::Operator* op = static_cast<AST::Operator*>(expression);
			if(op->OperatorType() == AST::OperatorKind::ASN) {
				return true;
			}
		}
		return false;
	}

}