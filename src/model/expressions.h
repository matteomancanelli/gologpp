#ifndef GOLOGPP_EXPRESSIONS_H_
#define GOLOGPP_EXPRESSIONS_H_

#include <memory>
#include "language.h"
#include "utilities.h"
#include "gologpp.h"

namespace gologpp {


enum ExpressionTypeTag {
	BOOLEAN_EXPRESSION,
	VALUE_EXPRESSION,
	STATEMENT,
};



string to_string(ExpressionTypeTag t);



class Expression : public virtual AbstractLanguageElement {
protected:
	Expression(Scope &parent_scope);
	Expression(const Expression &) = delete;
	Expression(Expression &&) = default;
	Expression &operator = (const Expression &) = delete;

public:
	virtual ~Expression() = default;

	Scope &parent_scope();
	const Scope &parent_scope() const;
	virtual bool is_ref() const;
	virtual string to_string(const string &pfx) const = 0;

	virtual ExpressionTypeTag expression_type_tag() const = 0;

protected:
	Scope &parent_scope_;
};


class Global : public Identifier, public std::enable_shared_from_this<Global> {
protected:
	Global(const string &name, const vector<shared_ptr<AbstractVariable>> &args);

	template<class GologT>
	Reference<GologT> *make_reference(Scope &parent_scope, const vector<Expression *> &args = {})
	{
		static_assert(std::is_base_of<Identifier, GologT>::value,
			"Cannot reference a type that is not derived from Identifier");
		return new Reference<GologT>(
			std::dynamic_pointer_cast<GologT>(this->shared_from_this()),
			parent_scope,
			std::move(args)
		);
	}


public:
	virtual ~Global() = default;

	vector<shared_ptr<AbstractVariable>> &args();
	const vector<shared_ptr<AbstractVariable>> &args() const;
	shared_ptr<AbstractVariable> argument(arity_t idx) const;

	virtual void compile(AExecutionContext &ctx) = 0;
	virtual Expression *ref(Scope &parent_scope, const vector<Expression *> &args) = 0;

private:
	vector<shared_ptr<AbstractVariable>> args_;
};


class BooleanExpression : public Expression {
protected:
	BooleanExpression(Scope &parent_scope);
	using Expression::Expression;

public:
	typedef BooleanExpression expression_t;
	virtual ExpressionTypeTag expression_type_tag() const override;
	static ExpressionTypeTag static_type_tag();
};


class NumericExpression : public Expression {
protected:
	NumericExpression(Scope &parent_scope);
	using Expression::Expression;

public:
	typedef NumericExpression expression_t;
	virtual ExpressionTypeTag expression_type_tag() const override;
	static ExpressionTypeTag static_type_tag();
};


class Statement : public Expression {
protected:
	Statement(Scope &parent_scope);
	using Expression::Expression;

public:
	typedef Statement expression_t;
	virtual ExpressionTypeTag expression_type_tag() const override;
	static ExpressionTypeTag static_type_tag();
};


}

#endif // GOLOGPP_EXPRESSIONS_H_
