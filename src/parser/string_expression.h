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

#ifndef GOLOGPP_PARSER_STRING_EXPRESSION_H_
#define GOLOGPP_PARSER_STRING_EXPRESSION_H_

#include "utilities.h"

namespace gologpp {
namespace parser {


struct StringExpressionParser : grammar<Expression *(Scope &)> {
	StringExpressionParser();

	rule<Expression *(Scope &)> expression;
	rule<Expression *(Scope &)> unary_expr;
	rule<Reference<Variable> *(Scope &)> var_ref;
	rule<Expression *(Scope &)> concatenation;
	rule<Expression *(Scope &)> conversion;
	rule<Expression *(Scope &)> convertible_expr;
	rule<Expression *(Scope &)> implicit_conversion;
};


} // namespace parser
} // namespace gologpp

#endif // GOLOGPP_PARSER_STRING_EXPRESSION_H_
