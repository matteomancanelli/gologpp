#ifndef GOLOGPP_ACTION_H_
#define GOLOGPP_ACTION_H_

#include <vector>
#include <set>
#include <memory>
#include <algorithm>

#include "gologpp.h"

#include "error.h"
#include "Formula.h"
#include "atoms.h"
#include "Language.h"
#include "Scope.h"

namespace gologpp {


class AbstractEffectAxiom;


class AbstractAction : public Statement, public Identifier, public virtual AbstractLanguageElement {
public:
	AbstractAction(const string &name, const vector<string> &arg_names);

	AbstractAction(const AbstractAction &) = delete;
	AbstractAction(AbstractAction &&other) = default;

	virtual ~AbstractAction() override = default;

	const vector<unique_ptr<AbstractEffectAxiom>> &effects() const;

	template<class T>
	void add_effect(T &&effect)
	{ effects_.push_back(unique_ptr<AbstractEffectAxiom>(new T(std::move(effect)))); }

	template<class ExpressionT>
	void declare_argument(const string &name)
	{
		if (std::find(args_.begin(), args_.end(), name) == args_.end())
			throw Bug("Variable `" + name + "' not defined for Action `" + this->name() + "'");
		scope_.variable<ExpressionT>(name);
	}

	const vector<string> &arg_names() const;

	shared_ptr<AbstractVariable> argument(arity_t idx) const;

	Scope &scope();
	const Scope &scope() const;

protected:
	Scope scope_;

	vector<unique_ptr<AbstractEffectAxiom>> effects_;
	vector<string> args_;
};


class Action : public AbstractAction, public LanguageElement<Action> {
public:
	using AbstractAction::AbstractAction;
	Action(const Action &) = delete;
	Action(Action &&) = default;

	const BooleanExpression &precondition() const;

	template<class T>
	void set_precondition(T &&precondition)
	{ precondition_ = unique_ptr<BooleanExpression>(new T(std::move(precondition))); }

	virtual void implement(Implementor &) override;

protected:
	unique_ptr<BooleanExpression> precondition_;
};


class ExogAction : public AbstractAction, public LanguageElement<ExogAction> {
public:
	using AbstractAction::AbstractAction;
	ExogAction(const Action &) = delete;
	ExogAction(ExogAction &&) = default;
	virtual void implement(Implementor &) override;
};


class AbstractTransition : public virtual AbstractLanguageElement {
public:
	AbstractTransition(shared_ptr<Action> action, vector<unique_ptr<AbstractConstant>> args);

	const Action &action() const;
	const vector<unique_ptr<AbstractConstant>> &args() const;

protected:
	shared_ptr<Action> action_;
	vector<unique_ptr<AbstractConstant>> args_;
};


class Transition : public AbstractTransition, public LanguageElement<Transition> {
public:
	using AbstractTransition::AbstractTransition;
	Transition(const Transition &) = delete;
	Transition(Transition &&) = delete;
	virtual void implement(Implementor &) override;
};


class ExogTransition : public AbstractTransition, public LanguageElement<ExogTransition> {
public:
	using AbstractTransition::AbstractTransition;
	ExogTransition(const ExogTransition &) = delete;
	ExogTransition(ExogTransition &&) = default;
	virtual void implement(Implementor &) override;
};



} // namespace gologpp



#endif /* GOLOGPP_ACTION_H_ */
