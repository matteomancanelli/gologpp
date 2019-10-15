/*************************************************************************
 * This file is part of golog++.
 *
 * golog++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * golog++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with golog++.  If not, see <https://www.gnu.org/licenses/>.
**************************************************************************/

#include "action.h"
#include "effect_axiom.h"
#include "scope.h"
#include "execution.h"
#include "variable.h"
#include "value.h"

#include <model/activity.h>

#include <eclipseclass.h>

#include <unordered_map>

namespace gologpp {


Semantics<Action>::Semantics(const Action &a)
: AbstractSemantics<Action>(a)
{}


EC_word Semantics<Action>::durative_action()
{
	element().scope().semantics().init_vars();

	vector<EC_word> arg_domains;
	for (const shared_ptr<Variable> &param : element().params())
		if (param->domain().is_defined())
			arg_domains.emplace_back(
				param->semantics<Variable>().member_restriction()
			);

	EC_word prim_action = ::term(EC_functor("durative_action", 1), plterm());

	if (arg_domains.size() > 0)
		return ::term(EC_functor(":-", 2),
			prim_action,
			to_ec_term(",", arg_domains)
		);
	else
		return prim_action;
}


EC_word Semantics<Action>::plterm()
{
	if (element().arity() > 0)
		return ::term(
			EC_functor(element().name().c_str(), element().arity()),
			to_ec_words(element().params()).data()
		);
	else
		return EC_atom(element().name().c_str());
}


vector<EC_word> Semantics<Action>::durative_causes_vals()
{
	vector<EC_word> rv;
	for (const unique_ptr<AbstractEffectAxiom> &effect : element().effects())
		rv.push_back(effect->semantics().plterm());
	return rv;
}


EC_word Semantics<Action>::durative_poss()
{
	element().scope().semantics().init_vars();
	return ::term(EC_functor("durative_poss", 2),
		plterm(),
		element().precondition().semantics().plterm()
	);
}


EC_word Semantics<Action>::senses()
{
	return ::term(EC_functor("senses", 2),
		plterm(),
		element().senses()->semantics().plterm()
	);
}


Semantics<ExogAction>::Semantics(const ExogAction &a)
: AbstractSemantics<ExogAction>(a)
{}


EC_word Semantics<ExogAction>::exog_action()
{
	element().scope().semantics().init_vars();
	return ::term(EC_functor("exog_action", 1), plterm());
}


EC_word Semantics<ExogAction>::plterm()
{
	if (element().arity() > 0)
		return ::term(
			EC_functor(element().name().c_str(), element().arity()),
			to_ec_words(element().params()).data()
		);
	else
		return EC_atom(element().name().c_str());
}


vector<EC_word> Semantics<ExogAction>::causes_vals()
{
	element().scope().semantics().init_vars();
	vector<EC_word> rv;
	for (const unique_ptr<AbstractEffectAxiom> &effect : element().effects())
		rv.push_back(effect->semantics().plterm());
	return rv;
}


EC_word Semantics<ExogAction>::poss()
{
	return ::term(EC_functor("poss", 2),
		plterm(),
		element().precondition().semantics().plterm()
	);
}



const Activity &Semantics<Activity>::activity()
{ return dynamic_cast<const Activity &>(element()); }


EC_word Semantics<Activity>::plterm()
{
	string state;

	switch (activity().state()) {
	case Activity::State::IDLE:
		state = "idle";
		break;
	case Activity::State::RUNNING:
		state = "running";
		break;
	case Activity::State::FINAL:
		state = "final";
		break;
	case Activity::State::FAILED:
		state = "failed";
		break;
	case Activity::State::PREEMPTED:
		state = "preempted";
	}

	return ::term(EC_functor("exog_state_change", 3),
		Semantics<Grounding<Action>>::plterm(),
		EC_word(ReadylogContext::instance().backend().time().time_since_epoch().count()),
		EC_atom(state.c_str())
	);
}


EC_word Semantics<Activity>::sensing_result()
{
	return ::term(EC_functor("e", 2),
		activity().target()->senses()->semantics().plterm(),
		activity().sensing_result()->semantics().plterm()
	);
}



const Transition &Semantics<Transition>::trans()
{ return dynamic_cast<const Transition &>(element()); }

EC_word Semantics<Transition>::plterm()
{
	string name;

	switch (trans().hook()) {
	case Transition::Hook::START:
		name = "start";
		break;
	case Transition::Hook::STOP:
		name = "stop";
		break;
	case Transition::Hook::FAIL:
		name = "fail";
		break;
	case Transition::Hook::FINISH:
		name = "finish";
	}

	return ::term(EC_functor(name.c_str(), 2),
		Semantics<Grounding<Action>>::plterm(),
		EC_word(ReadylogContext::instance().backend().time().time_since_epoch().count())
	);
}




} // namespace gologpp
