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

#ifndef GOLOGPP_PARSER_UTILITIES_H_
#define GOLOGPP_PARSER_UTILITIES_H_

#include <model/expressions.h>

#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>

#include <boost/preprocessor/seq/for_each.hpp>


namespace gologpp {
namespace parser {


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace spirit = boost::spirit;
namespace attr = boost::spirit;


using namespace boost::spirit;
using namespace boost::spirit::qi;
using namespace boost::spirit::qi::labels;
using namespace boost::phoenix;


using Typename = string;


/******************
* Little helpers
******************/


template<class T1, class T2>
#ifdef BOOST_FUSION_HAS_VARIADIC_VECTOR
using fusion_wtf_vector = boost::fusion::vector<T1, T2>;
#else
using fusion_wtf_vector = boost::fusion::vector2<T1, T2>;
#endif


using iterator = boost::spirit::line_pos_iterator<string::const_iterator>;



struct gologpp_skipper : public qi::grammar<iterator> {
	gologpp_skipper();
	
	qi::rule<iterator> skip;
	qi::rule<iterator> spc;
	qi::rule<iterator> comment_multiline;
	qi::rule<iterator> comment_oneline;
	qi::rule<iterator> preprocessor;
};


template<typename... SignatureTs>
using rule = boost::spirit::qi::rule<iterator, gologpp_skipper, SignatureTs...>;

template<typename... SignatureTs>
using grammar = boost::spirit::qi::grammar<iterator, gologpp_skipper, SignatureTs...>;



rule<string()> &r_name();

rule<string()> &raw_string_literal();


string get_error_context(
	const iterator &begin,
	const iterator &errpos,
	const iterator &end
);

void handle_error(
	const iterator &begin,
	const iterator &errpos,
	const iterator &end,
	const boost::spirit::info &expected
);


#ifdef GOLOGPP_DEBUG_PARSER
#define GOLOGPP_DEBUG_NODE(n) debug(n);
#define GOLOGPP_DEBUG_NODE_A(_r, _data, node) GOLOGPP_DEBUG_NODE(node)
#define GOLOGPP_DEBUG_NODES(n) BOOST_PP_SEQ_FOR_EACH(GOLOGPP_DEBUG_NODE_A, (), n)
#else
#define GOLOGPP_DEBUG_NODE(n)
#define GOLOGPP_DEBUG_NODES(n)
#endif


} // namespace parser
} // namespace gologpp


#endif // GOLOGPP_PARSER_UTILITIES_H_

