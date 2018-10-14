#ifndef GOLOGPP_GRAMMAR_H_
#define GOLOGPP_GRAMMAR_H_

#include "utilities.h"
#include "statements.h"
#include "fluent.h"
#include "action.h"
#include "functions.h"

#include <boost/spirit/include/qi_kleene.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/qi_alternative.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_expect.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/home/qi/nonterminal/error_handler.hpp>

namespace gologpp {
namespace parser {


struct ProgramParser : grammar<Statement *(Scope &)> {
	ProgramParser()
	: ProgramParser::base_type(program)
	{
		program = expect[*( omit[ // Discard attributes, they just register themselves as Globals
			numeric_fluent(_r1)
			| boolean_fluent(_r1)
			| symbolic_fluent(_r1)
			| action(_r1)
			| function(_r1)
		] ) >> statement(_r1) >> eoi];

		on_error<rethrow>(program,
			phoenix::bind(&handle_error, _1, _3, _2, _4)
		);

		BOOST_SPIRIT_DEBUG_NODE(program);
	}

	rule<Statement *(Scope &)> program;
	ActionParser action;
	AbstractFunctionParser function;
	StatementParser statement;
};



} // namespace parser
} // namespace gologpp


#endif // GOLOGPP_GRAMMAR_H_
