#ifndef GOLOGPP_LANGUAGE_H_
#define GOLOGPP_LANGUAGE_H_

#include "gologpp.h"
#include <memory>
#include "semantics.h"
#include "types.h"

namespace gologpp {


class AbstractLanguageElement {
public:
	AbstractLanguageElement() = default;

	// By default, a language element is not copyable, since it is part of a graph
	// that may even contain cycles.
	AbstractLanguageElement(const AbstractLanguageElement &) = delete;
	AbstractLanguageElement(AbstractLanguageElement &&) = delete;
	AbstractLanguageElement &operator = (const AbstractLanguageElement &) = delete;
	AbstractLanguageElement &operator = (AbstractLanguageElement &&) = delete;

	virtual ~AbstractLanguageElement() = default;

	template<class GologT = AbstractLanguageElement>
	Semantics<GologT> &semantics() const
	{ return dynamic_cast<Semantics<GologT> &>(*semantics_); }

	void set_implementation(unique_ptr<AbstractSemantics> &&impl);
	virtual void attach_semantics(SemanticsFactory &) = 0;

	virtual string to_string(const string &pfx) const = 0;
	string str() const;

	virtual Scope &scope() = 0;
	virtual const Scope &scope() const = 0;
	virtual Scope &parent_scope() = 0;
	virtual const Scope &parent_scope() const = 0;

	bool set_type_by_name(const string &name);
	bool set_type(const Type &t);
	virtual const Type &type() const;

	// Unambiguous alias name to simplify type resolution for phoenix::bind in the parser
	Scope &m_scope();

	bool is_ref() const;

	template<class T>
	bool is_a() const
	{ return dynamic_cast<const T *>(this); }

	template<class T>
	void ensure_type();



protected:
	unique_ptr<AbstractSemantics> semantics_;
	shared_ptr<const Type> type_;
};


template<class GologT>
class LanguageElement : public virtual AbstractLanguageElement {
public:
	typedef GologT golog_t;

	virtual ~LanguageElement() = default;

	template<class = GologT>
	Semantics<GologT> &semantics() const
	{ return static_cast<Semantics<GologT> &>(*semantics_); }
};



} // namespace gologpp

#endif // GOLOGPP_LANGUAGE_H_
