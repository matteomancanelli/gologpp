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

#ifndef GOLOGPP_SCOPE_H_
#define GOLOGPP_SCOPE_H_

#include <vector>
#include <unordered_map>
#include <boost/optional.hpp>
#include <cassert>

#include "utilities.h"
#include "language.h"
#include "expressions.h"
#include "error.h"
#include "semantics.h"

#include "gologpp.h"

namespace gologpp {


Scope &global_scope();



/**
 * @brief The NoScopeOwner class: A language element that does not open a new scope,
 * i.e. its scope and the scope of its children is its parent scope.
 */
class NoScopeOwner : public virtual AbstractLanguageElement {
public:
	virtual const Scope &scope() const override;
	virtual Scope &scope() override;
};



/**
 * @brief The ScopeOwner class: A language element that opens a new scope.
 * I.e. its scope is the parent scope of its children, which different from its parent scope.
 */
class ScopeOwner : public virtual AbstractLanguageElement {
public:
	ScopeOwner(Scope *owned_scope);

	virtual ~ScopeOwner() override = default;

	virtual const Scope &scope() const override;
	virtual Scope &scope() override;

protected:
	unique_ptr<Scope> scope_;
};



/**
 * @brief The Scope class: A container and definition space for language elements.
 * The global (root) scope is its own parent.
 */
class Scope : public LanguageElement<Scope> {
private:
	Scope();

	template<class K, class V>
	using unordered_map = std::unordered_map<K, V>;

	void init_types();

public:
	using VariablesMap = unordered_map<string, shared_ptr<Variable>>;
	using GlobalsMap = unordered_map<Identifier, shared_ptr<Global>>;
	using DomainsMap = unordered_map<Name, shared_ptr<Domain>>;
	using TypesMap = unordered_map<Name, shared_ptr<const Type>>;

	explicit Scope(AbstractLanguageElement &owner, const vector<shared_ptr<Variable>> &lookup_vars = {});
	Scope(Scope &parent_scope);

	~Scope() override;

	shared_ptr<Variable> get_var(VarDefinitionMode var_def_mode, const Type &type, const string &name);
	shared_ptr<Variable> lookup_var(const string &name) const;
	bool has_var(const string &name) const;
	vector<shared_ptr<Variable>> lookup_vars(const vector<string> &names);
	vector<shared_ptr<Variable>> vars() const;

	void attach_semantics(SemanticsFactory &implementor) override;

	static Scope &global_scope()
	{ return global_scope_; }

	virtual Scope &scope() override;
	virtual const Scope &scope() const override;

	virtual Scope &parent_scope() override;
	virtual const Scope &parent_scope() const override;

	void set_owner(AbstractLanguageElement *owner);
	const AbstractLanguageElement *owner() const;
	AbstractLanguageElement *owner();

	template<class GologT = Global>
	shared_ptr<GologT> lookup_global(const string &name, arity_t arity)
	{
		if (exists_global(name, arity))
			return std::dynamic_pointer_cast<GologT>(
				(*globals_) [{ name, arity }]
			);
		else
			return shared_ptr<GologT>();
	}


	const VariablesMap &var_map() const;
	void implement_globals(SemanticsFactory &implementor, AExecutionContext &ctx);
	void clear();

	bool exists_global(const string &name, arity_t arity) const;


	template<class GologT>
	GologT *declare_global(
		Scope *own_scope,
		const Type &type,
		const string &name,
		const boost::optional<vector<shared_ptr<Variable>>> &args
	) {
		GologT *rv = nullptr;
		arity_t arity = static_cast<arity_t>(args.get_value_or({}).size());
		if (exists_global(name, arity)) {
			rv = lookup_global<GologT>(name, arity).get();

			if (rv && !(rv->type() <= type))
				throw TypeError("Cannot redeclare " + rv->str()
					+ " with type " + type.name());

			// Here be dragons:
			// own_scope is constructed by the parser, and it may or may not
			// be the one that is already in use by the existing Global.
			// If it's not, we have to delete it since it won't be used.
			if (&(rv->scope()) != own_scope)
				delete own_scope;
		}
		else
			(*globals_) [ { name, arity } ]
				.reset(new GologT(own_scope, type, name, args.get_value_or({})));

		return lookup_global<GologT>(name, arity).get();
	}


	template<class GologT, class... DefinitionTs>
	GologT *define_global(
		Scope *own_scope,
		const Type &type,
		const string &name,
		const boost::optional<vector<shared_ptr<Variable>>> &args,
		DefinitionTs... definition_args
	) {
		GologT *rv = nullptr;
		arity_t arity = static_cast<arity_t>(args.get_value_or({}).size());
		if (exists_global(name, arity)) {
			rv = lookup_global<GologT>(name, arity).get();

			if (rv && !(rv->type() <= type))
				throw TypeError("Cannot redefine " + rv->str()
					+ " with type " + type.name());

			// Here be dragons:
			// own_scope is constructed by the parser, and it may or may not
			// be the one that is already in use by the existing Global.
			// If it's not, we have to delete it since it won't be used.
			if (&(rv->scope()) != own_scope)
				delete own_scope;

			// TODO: if (rv->defined()): Warn on redefinition
		}
		else
			rv = declare_global<GologT>(own_scope, type, name, args);
		rv->define(definition_args...);
		return rv;
	}



	bool exists_type(const string &name) const;

	template<class TypeT = Type>
	shared_ptr<const TypeT> lookup_type(const string &name) const
	{
		if (exists_type(name))
			return std::dynamic_pointer_cast<const TypeT>(
				types_->find(name)->second
			);
		else
			return nullptr;
	}

	shared_ptr<const ListType> lookup_list_type(const Type &elem_type) const;

	const Type *lookup_type_raw(const string &name);

	void register_type_raw(Type *t);
	void register_type(shared_ptr<Type> t);

	void register_global(Global *g);

	bool exists_domain(const string &name) const;
	shared_ptr<Domain> lookup_domain(const string &name, const Type &t);

	void register_domain_raw(Domain *d);
	void register_domain(const shared_ptr<Domain> &d);
	void declare_domain(const string &name, const Type &type);
	void define_domain(const string &name, const Type &type, const Domain &input);

	Value *get_symbol(const string &name);

	virtual string to_string(const string &pfx) const override;

	vector<shared_ptr<Global>> globals() const;


private:
	static Scope global_scope_;
	Scope &parent_scope_;
	AbstractLanguageElement *owner_;
	VariablesMap variables_;

	shared_ptr<GlobalsMap> globals_;
	shared_ptr<DomainsMap> domains_;
	shared_ptr<TypesMap> types_;
};



template<class T>
const T &type()
{
	if (!global_scope().exists_type(T::static_name()))
		global_scope().register_type_raw(new T());
	return *global_scope().lookup_type<T>(T::static_name());
}


template<class T>
const T &get_type()
{ return type<T>(); }


} // namespace gologpp







#endif
