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

#include "value.h"
#include <model/value.h>

namespace gologpp {

Semantics<Value>::Semantics(const Value &value)
: value_(value)
{}

EC_word Semantics<Value>::plterm()
{
	if (value_.type().is<BoolType>()) {
		if (static_cast<bool>(value_))
			return EC_atom("true");
		else
			return EC_atom("fail");
	}
	else if (value_.type().is<NumberType>()) {
		if (value_.representation().type() == boost::typeindex::type_id<double>())
			return EC_word(static_cast<double>(value_));
		else if (value_.representation().type() == boost::typeindex::type_id<int>())
			return EC_word(static_cast<int>(value_));
		else if (value_.representation().type() == boost::typeindex::type_id<long>())
			return EC_word(static_cast<long>(value_));
		else if (value_.representation().type() == boost::typeindex::type_id<unsigned int>())
			return EC_word(static_cast<long>(static_cast<unsigned int>(value_)));
		else if (value_.representation().type() == boost::typeindex::type_id<unsigned long>())
			return EC_word(static_cast<long long>(static_cast<unsigned long>(value_)));
		else
			throw std::runtime_error("Unknown Constant type");
	}
	else if (
		value_.type().is<SymbolType>()
		|| value_.type().is<StringType>()
	)
		return EC_atom(static_cast<string>(value_).c_str());
	else if (value_.type().is<CompoundType>()) {
		EC_word field_list = ::nil();
		for (auto &pair : static_cast<const CompoundType::Representation &>(value_))
			field_list = ::list(
				::term(EC_functor(pair.first.c_str(), 1),
					pair.second->semantics().plterm()
				),
				field_list
			);
		return ::term(EC_functor("gpp_compound", 2),
			EC_atom(("#" + value_.type_name()).c_str()),
			field_list
		);
	}
	else if (value_.type().is<ListType>()) {
		EC_word list = ::nil();

		const ListType::Representation &list_repr = static_cast<const ListType::Representation &>(value_);
		ListType::Representation::const_reverse_iterator it = list_repr.rbegin();
		while (it != list_repr.rend())
			list = ::list((*it++)->semantics().plterm(), list);
		return ::term(EC_functor("gpp_list", 2),
			EC_atom(("#" + dynamic_cast<const ListType &>(value_.type()).element_type().name()).c_str()),
			list
		);
	}
	else if (value_.type().is<UndefinedType>()) {
		return EC_atom("fail");
	}
	else
		throw std::runtime_error("Unknown Constant type");
}


} // namespace gologpp
