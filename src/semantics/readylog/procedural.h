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

#ifndef READYLOG_PROCEDURE_H_
#define READYLOG_PROCEDURE_H_

#include "semantics.h"
#include "scope.h"
#include "variable.h"
#include "utilities.h"
#include "fluent.h"
#include "reference.h"

#include <model/semantics.h>
#include <model/expressions.h>
#include <model/procedural.h>
#include <model/scope.h>

#include <model/user_error.h>

#include <type_traits>

#include <eclipseclass.h>

namespace gologpp {



template<>
class Semantics<Function> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Function &function)
	: function_(function)
	{}

	virtual EC_word plterm() override;
	virtual EC_word definition();
	EC_word return_var();

private:
	const Function &function_;
	EC_word return_var_;
};



template<>
class Semantics<Block> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Block &);
	virtual EC_word plterm() override;
	EC_word current_program();
	void set_current_program(EC_word e);

private:
	const Block &block_;
	ManagedTerm current_program_;
};



template<>
class Semantics<Choose> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Choose &);
	virtual EC_word plterm() override;

private:
	const Choose &choose_;
};



template<>
class Semantics<Conditional> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Conditional &);
	virtual EC_word plterm() override;

private:
	const Conditional &conditional_;
};



template<>
class Semantics<Concurrent> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Concurrent &);
	virtual EC_word plterm() override;

private:
	const Concurrent &concurrent_;
};



template<class LhsT>
class Semantics<Assignment<LhsT>> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Assignment<LhsT> &ass)
	: assignment_(ass)
	{
		throw std::runtime_error(string("Assignment to ") + typeid(LhsT).name() + " is not implemented");
	}

	virtual EC_word plterm() override
	{ throw std::runtime_error(string("Assignment to ") + typeid(LhsT).name() + " is not implemented"); }

private:
	const Assignment<LhsT> &assignment_;
};



template<>
class Semantics<Assignment<Reference<Fluent>>> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Assignment<Reference<Fluent>> &ass);

	virtual EC_word plterm() override;

private:
	const Assignment<Reference<Fluent>> &assignment_;
};



template<>
class Semantics<Assignment<FieldAccess>> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Assignment<FieldAccess> &ass);

	virtual EC_word plterm() override;

private:
	const Assignment<FieldAccess> &assignment_;
	const FieldAccess &field_access_;
};



template<>
class Semantics<Assignment<ListAccess>> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Assignment<ListAccess> &ass);

	virtual EC_word plterm() override;

private:
	const Assignment<ListAccess> &assignment_;
	const ListAccess &field_access_;
};



template<>
class Semantics<Pick> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Pick &pick);
	virtual EC_word plterm() override;

private:
	const Pick &pick_;
};



template<>
class Semantics<Search> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Search &);
	virtual EC_word plterm() override;

private:
	const Search &search_;
};



template<>
class Semantics<Solve> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Solve &);
	virtual EC_word plterm() override;

private:
	const Solve &solve_;
};



template<>
class Semantics<Test> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Test &);
	virtual EC_word plterm() override;

private:
	const Test &test_;
};



template<>
class Semantics<While> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const While &);
	virtual EC_word plterm() override;

private:
	const While &while_;
};



template<>
class Semantics<Return> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const Return &r);
	virtual EC_word plterm() override;

private:
	const Return &ret_;
};



template<>
class Semantics<DurativeCall> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const DurativeCall &call);
	virtual EC_word plterm() override;

private:
	const DurativeCall &call_;
};


std::pair<const Reference<Fluent> *, EC_word>
traverse_mixed_field_access(const FieldAccess *fa, const ListAccess *la);


template<>
class Semantics<FieldAccess> : public Semantics<Expression> {
public:
	Semantics(const FieldAccess &field_access);

	virtual EC_word plterm() override;
	EC_word field_assign(const Expression &value);
	EC_atom pl_field_name();
	void set_lvalue(bool lvalue);

private:
	const FieldAccess &field_access_;
	bool is_lvalue_;
};



template<>
class Semantics<ListAccess> : public Semantics<Expression> {
public:
	Semantics(const ListAccess &list_access);

	virtual EC_word plterm() override;
	EC_word pl_index();

private:
	const ListAccess &list_access_;
};



template<>
class Semantics<ListPop> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const ListPop &list_access);

	virtual EC_word plterm() override;

private:
	const ListPop &list_pop_;
};



template<>
class Semantics<ListPush> : public Semantics<AbstractLanguageElement> {
public:
	Semantics(const ListPush &list_access);

	virtual EC_word plterm() override;

private:
	const ListPush &list_push_;
};



template<>
class Semantics<ListLength> : public Semantics<Expression> {
public:
	Semantics(const ListLength &list_access);

	virtual EC_word plterm() override;

private:
	const ListLength &list_length_;
};



} // namespace gologpp


#endif // READYLOG_PROCEDURE_H_
