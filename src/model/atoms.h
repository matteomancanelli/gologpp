#ifndef GOLOGPP_ATOMS_H_
#define GOLOGPP_ATOMS_H_

#include "gologpp.h"

#include "utilities.h"
#include "expressions.h"
#include "language.h"

#include <memory>

namespace gologpp {


Scope &global_scope();


class AbstractVariable : public Identifier, public virtual AbstractLanguageElement {
public:
	AbstractVariable(const string &name);

	virtual ExpressionTypeTag expression_type_tag() const = 0;

	template<class ExpressionT>
	operator Variable<ExpressionT> *() {
		return dynamic_cast<Variable<ExpressionT> *>(this);
	}

	template<class ExpressionT>
	operator ExpressionT *() {
		return static_cast<ExpressionT *>(dynamic_cast<Variable<ExpressionT> *>(this));
	}
};


template<class ExpressionT>
class Variable
: public ExpressionT
, public LanguageElement<Variable<ExpressionT>>
, public AbstractVariable
, public std::enable_shared_from_this<Variable<ExpressionT>>
{
protected:
	Variable(const string &name, Scope &parent_scope)
	: ExpressionT(parent_scope)
	, AbstractVariable(name)
	{}

public:
	Variable(Variable<ExpressionT> &&) = default;
	Variable(const Variable<ExpressionT> &) = delete;
	Variable<ExpressionT> &operator = (Variable<ExpressionT> &&) = default;
	Variable<ExpressionT> &operator = (const Variable<ExpressionT> &) = delete;

	friend Scope;

	virtual ExpressionTypeTag expression_type_tag() const override
	{ return ExpressionT::expression_type_tag(); }

	Reference<Variable<ExpressionT>> *ref(Scope &parent_scope)
	{
		return new Reference<Variable<ExpressionT>>(
			this->shared_from_this(),
			parent_scope,
			{}
		);
	}


	virtual ~Variable() override = default;
	DEFINE_IMPLEMENT
};


class AbstractConstant : public virtual AbstractLanguageElement {
public:
	AbstractConstant(const string &representation);

	const string &representation() const;

protected:
	const string representation_;
};


template<class ExpressionT>
class Constant
: public ExpressionT
, public AbstractConstant
, public LanguageElement<Constant<ExpressionT>>
{
public:
	Constant(const string &representation)
	: ExpressionT(global_scope())
	, AbstractConstant(representation)
	{}

	template<class NumT>
	Constant(NumT n)
	: ExpressionT(global_scope())
	, AbstractConstant(std::to_string(n))
	{}

	Constant(Constant<ExpressionT> &&) = default;
	Constant(const Constant<ExpressionT> &) = delete;
	Constant<ExpressionT> &operator = (Constant<ExpressionT> &&) = default;
	Constant<ExpressionT> &operator = (const Constant<ExpressionT> &) = delete;

	virtual ~Constant() override = default;

	virtual ExpressionTypeTag expression_type_tag() const override
	{ return ExpressionT::expression_type_tag(); }

	DEFINE_IMPLEMENT
};


} // namespace gologpp



namespace std {

template<>
struct hash<shared_ptr<gologpp::AbstractVariable>> {
    size_t operator () (const shared_ptr<gologpp::AbstractVariable> &o) const
    { return o->hash(); }
};

} // namespace std

#endif
