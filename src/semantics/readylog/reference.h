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

#ifndef READYLOG_REFERENCE_H_
#define READYLOG_REFERENCE_H_

#include <model/scope.h>
#include <model/reference.h>

// Include headers for everything we might have a reference to (see plterm() method)
#include <model/action.h>
#include <model/fluent.h>
#include <model/procedural.h>

#include "semantics.h"
#include "utilities.h"
#include "variable.h"

#include <eclipseclass.h>


namespace gologpp {

template<class GologT> class Reference;



template<class ArgsT>
class Semantics<TBinding<ArgsT>>
: public Semantics<AbstractLanguageElement>
, public AbstractSemantics<TBinding<ArgsT>>
{
public:
	using AbstractSemantics<TBinding<ArgsT>>::AbstractSemantics;

	virtual EC_word plterm() override
	{
		vector<EC_word> pl_binds;
		for (const auto &pval : this->element().map()) {
			pval.first->semantics().init();
			pl_binds.push_back(
				::term(EC_functor("=", 2),
					pval.first->semantics().plterm(),
					pval.second.get().semantics().plterm()
				)
			);
		}

		return to_ec_term(",", pl_binds);
	}
};



template<>
class Semantics<Reference<Variable>>
: public Semantics<Expression>
, public AbstractSemantics<Reference<Variable>>
{
public:
	Semantics(const Reference<Variable> &ref);

	virtual EC_word plterm() override;
};



template<class GologT, class ExprT>
EC_word reference_term(const ReferenceBase<GologT, ExprT> &ref)
{
	if (ref.arity() > 0)
		return ::term(EC_functor(ref.name().c_str(), ref.arity()),
			to_ec_words(ref.args()).data()
		);
	else
		return EC_atom(ref.name().c_str());
}



template<class TargetT>
class Semantics<Reference<TargetT>>
: public AbstractSemantics<Reference<TargetT>>
, public Semantics<typename TargetT::SignifierT>
{
public:
	using AbstractSemantics<Reference<TargetT>>::AbstractSemantics;

	const Reference<TargetT> &ref()
	{ return this->element(); }

	virtual EC_word plterm() override
	{ return reference_term(ref()); }

	bool args_need_eval() {
		for (const unique_ptr<Expression> &expr : ref().args())
			if (!expr->is_a<Reference<Variable>>() && !expr->is_a<Value>())
				return true;

		return false;
	}

	EC_word plterm_free_args()
	{
		if (ref().arity() == 0)
			return EC_atom(ref().name().c_str());
		else {
			vector<EC_word> args;
			arity_t i = 0;
			for (const unique_ptr<Expression> &expr : ref().args()) {
				if (!expr->is_a<Reference<Variable>>() && !expr->is_a<Value>())
					args.push_back(ref().target()->parameter(i)->semantics().plterm());
				else
					args.push_back(expr->semantics().plterm());
				++i;
			}

			return ::term(EC_functor(ref().name().c_str(), ref().arity()),
				args.data()
			);
		}
	}

	EC_word args_binding() {
		EC_word list = ::nil();
		for (const shared_ptr<Variable> &param : ref().target()->params()) {
			const Expression &arg = ref().arg_for_param(param);
			if (!arg.is_a<Reference<Variable>>() && !arg.is_a<Value>()) {
				list = ::list(
					::term(EC_functor("=", 2),
						param->semantics().plterm(),
						arg.semantics().plterm()
					),
					list
				);
			}
		}

		return ::term(EC_functor("and", 1), list);
	}
};


template<>
EC_word Semantics<Reference<Action>>::plterm();




} // namespace gologpp



#endif // READYLOG_REFERENCE_H_
