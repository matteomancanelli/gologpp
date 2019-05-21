#include "action.h"
#include "effect_axiom.h"
#include "execution.h"
#include "error.h"
#include <string>

namespace gologpp {


AbstractAction::AbstractAction(Scope *own_scope, const string &name, const vector<shared_ptr<Variable>> &args)
: Global(name, args)
, ScopeOwner(own_scope)
{
	set_precondition(new Constant(Bool::static_name(), true));
	vector<Expression *> mapping_args;
	for (const shared_ptr<Variable> &arg : args)
		mapping_args.push_back(new Reference<Variable>(arg));
	mapping_.reset(new ActionMapping(name, mapping_args));
}


AbstractAction::AbstractAction(Scope &parent_scope, const string &name)
: AbstractAction(new Scope(parent_scope), name, {})
{}

const vector<unique_ptr<AbstractEffectAxiom>> &AbstractAction::effects() const
{ return effects_; }


void AbstractAction::add_effect(AbstractEffectAxiom *effect)
{
	effect->set_action(*this);
	effects_.emplace_back(effect);
}

void AbstractAction::compile(AExecutionContext &ctx)
{ ctx.compile(*this); }


string AbstractAction::to_string(const string &pfx) const
{
	return name() + '(' + concat_list(args(), ", ", "") + ")" + " {"
		+ linesep + pfx + "precondition:" + linesep + precondition().to_string(pfx + indent)
		+ linesep + pfx + "effect:" + linesep + concat_list(effects(), ";" linesep, pfx) + linesep "}";
}

bool AbstractAction::operator != (const AbstractAction &other) const
{ return !(*this == other); }

bool AbstractAction::operator == (const AbstractAction &other) const
{ return typeid(*this) == typeid(other) && hash() == other.hash(); }

const Expression &AbstractAction::precondition() const
{ return *precondition_; }


void AbstractAction::set_precondition(Expression *cond)
{
	precondition_ = cond;
	precondition_->set_parent(this);
}

const ActionMapping& AbstractAction::mapping() const
{ return *mapping_; }

ActionMapping& AbstractAction::mapping()
{ return *mapping_; }

void AbstractAction::set_mapping(ActionMapping *mapping)
{ mapping_.reset(mapping); }


void AbstractAction::attach_semantics(SemanticsFactory &implementor)
{
	scope().attach_semantics(implementor);
	precondition_->attach_semantics(implementor);
	for (unique_ptr<AbstractEffectAxiom> &effect : effects_)
		effect->attach_semantics(implementor);
}




void Action::attach_semantics(SemanticsFactory &f)
{
	if (!semantics_) {
		semantics_ = f.make_semantics(*this);
		AbstractAction::attach_semantics(f);
		if (senses_)
			senses_->attach_semantics(f);
	}
}

string Action::to_string(const string &pfx) const
{ return linesep + pfx + "action " + AbstractAction::to_string(pfx); }

Reference<Action> *Action::make_ref(const vector<Expression *> &args)
{ return make_ref_<Action>(args); }

Expression *Action::ref(const vector<Expression *> &args)
{ return make_ref(args); }

void Action::set_senses(Reference<Fluent> *f)
{ senses_.reset(f); }

unique_ptr<Reference<Fluent>> &Action::senses()
{ return senses_; }

const unique_ptr<Reference<Fluent>> &Action::senses() const
{ return senses_; }


void Action::define(
	boost::optional<Expression *> precondition,
	boost::optional<vector<AbstractEffectAxiom *>> effects,
	boost::optional<Reference<Fluent> *> senses
) {
	if (precondition)
		set_precondition(precondition.value());
	if (effects)
		for (AbstractEffectAxiom *e : *effects)
			add_effect(e);
	if (senses)
		set_senses(senses.value());
}



void ExogAction::attach_semantics(SemanticsFactory &f)
{
	if (!semantics_) {
		semantics_ = f.make_semantics(*this);
		AbstractAction::attach_semantics(f);
	}
}

string ExogAction::to_string(const string &pfx) const
{ return linesep + pfx + "exog_action " + AbstractAction::to_string(pfx); }

Reference<ExogAction> *ExogAction::make_ref(const vector<Expression *> &args)
{ return make_ref_<ExogAction>(args); }

Expression *ExogAction::ref(const vector<Expression *> &args)
{ return make_ref(args); }


void ExogAction::define(
	boost::optional<Expression *> precondition,
	boost::optional<vector<AbstractEffectAxiom *>> effects
) {
	if (precondition)
		set_precondition(precondition.value());
	if (effects)
		for (AbstractEffectAxiom *e : *effects)
			add_effect(e);
}



} // namespace gologpp
