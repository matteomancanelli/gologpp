#ifndef GOLOGPP_PARSER_TYPES_H_
#define GOLOGPP_PARSER_TYPES_H_

#include <model/gologpp.h>
#include <model/types.h>

#include "utilities.h"


namespace gologpp {
namespace parser {


struct TypeDefinitionParser : public grammar<Type *(Scope &)> {
	TypeDefinitionParser();

	rule<Type *(Scope &)> type_definition;
	rule<CompoundType *(Scope &)> compound_type_def;
	rule<ListType *(Scope &)> list_type_def;
};


rule<Typename()> &any_type_specifier();



} // namespace parser
} // namespace gologpp


#endif // GOLOGPP_PARSER_TYPES_H_
