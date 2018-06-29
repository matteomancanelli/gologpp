#ifndef GOLOGPP_GRAMMAR_H_
#define GOLOGPP_GRAMMAR_H_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/qi_omit.hpp>

#include <boost/phoenix/object/construct.hpp>
#include <boost/phoenix/object/new.hpp>
#include <boost/phoenix/object/delete.hpp>
#include <boost/phoenix/operator/self.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/phoenix/bind/bind_function.hpp>

#include <boost/variant.hpp>

#include <string>
#include <vector>
#include <memory>

#include <model/gologpp.h>
#include <model/effect_axiom.h>
#include <model/action.h>
#include <model/fluent.h>
#include <model/procedural.h>
#include <model/reference.h>
#include <model/atoms.h>

namespace gologpp {
namespace parser {


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace spirit = boost::spirit;
namespace attr = boost::spirit;


using namespace boost::spirit;
using namespace boost::spirit::qi::labels;
using namespace boost::phoenix;


template<typename ResultT>
using rule = boost::spirit::qi::rule<string::const_iterator, ResultT, ascii::space_type>;

template<typename ResultT>
using grammar = boost::spirit::qi::grammar<string::const_iterator, ResultT, ascii::space_type>;


template<class T>
static inline auto l(T x)
{ return qi::lit(x); }

template<class T>
static inline auto _r(T &&x)
{ return boost::phoenix::ref(std::forward<T>(x)); }


template<class ExpressionT>
static auto type_mark();

template<>
auto type_mark<BooleanExpression>()
{ return l('?'); }

template<>
auto type_mark<NumericExpression>()
{ return l('%'); }



static rule<string()> r_name = qi::lexeme [
	qi::alpha >> *(qi::alnum | qi::char_('_'))
];


template<class ExpressionT>
struct VariableParser : grammar<shared_ptr<Variable<ExpressionT>>(Scope &)> {
	VariableParser(): VariableParser::base_type(variable) {
		variable = type_mark<ExpressionT>() >> r_name [
			_val = bind(&Scope::variable<ExpressionT>, _r1, _1)
		];
	}

	rule<shared_ptr<Variable<ExpressionT>>(Scope &)> variable;
};

static VariableParser<BooleanExpression> bool_var;
static VariableParser<NumericExpression> num_var;



static rule<BooleanConstant *()> bool_constant = (
	qi::string("true") | qi::string("false")
) [
	_val = new_<BooleanConstant>(_1)
];


static rule<NumericConstant *()> num_constant = float_ [
	_val = new_<NumericConstant>(_1)
];

static rule<AbstractConstant *()> constant = bool_constant | num_constant;

static rule<shared_ptr<AbstractVariable> (Scope &)> abstract_var =
		bool_var(_r1) [ _val = phoenix::bind(
			&std::dynamic_pointer_cast<AbstractVariable, BooleanVariable>,
			_1
		) ]
		| num_var(_r1) [ _val = phoenix::bind(
			&std::dynamic_pointer_cast<AbstractVariable, NumericVariable>,
			_1
		) ];

static rule<Expression *(Scope &)> atom =
		bool_var(_r1) [ _val = new_<Reference<BooleanVariable>>(_1, _r1) ]
		| num_var(_r1) [ _val = new_<Reference<NumericVariable>>(_1, _r1) ]
		| bool_constant
		| num_constant;


template<class ExpressionT>
struct UnboundReferenceParser : grammar<Reference<ExpressionT> *(Scope &)> {
	UnboundReferenceParser() : UnboundReferenceParser::base_type(pred_ref)
	{
		pred_ref = (r_name >> "(" >> ( (
			attr_cast<Expression *>(pred_ref(_r1))
			| attr_cast<Expression *>(atom(_r1))
		) %  ",") > ")"
		) [
			_val = new_<Reference<ExpressionT>>(_1, _r1, _2)
		];
	}

	rule<Reference<ExpressionT> *(Scope &)> pred_ref;
};



struct NumericExpressionParser : grammar<NumericExpression *(Scope &)> {
	NumericExpressionParser() : NumericExpressionParser::base_type(expression)
	{
		expression = num_constant | num_var_ref(_r1) | num_reference(_r1) | operation(_r1);
		num_var_ref = num_var(_r1) [ _val = new_<Reference<NumericVariable>>(_1, _r1) ];
		brace = '(' > expression(_r1) > ')';

		operation = ((expression(_r1) >> arith_operator) > expression(_r1)) [
			_val = new_<ArithmeticOperation>(at_c<0>(_1), at_c<1>(_1), _2)
		];

		arith_operator =
			qi::string("+") [ _val = val(ArithmeticOperation::ADDITION) ]
			| qi::string("-") [ _val = val(ArithmeticOperation::SUBTRACTION) ]
			| qi::string("/") [ _val = val(ArithmeticOperation::DIVISION) ]
			| qi::string("*") [ _val = val(ArithmeticOperation::MULTIPLICATION) ]
			| qi::string("**") [ _val = val(ArithmeticOperation::POWER) ]
			| qi::string("%") [ _val = val(ArithmeticOperation::MODULO) ]
		;
	}

	rule<NumericExpression *(Scope &)> expression;
	rule<NumericExpression *(Scope &)> operation;
	rule<NumericExpression *(Scope &)> brace;
	rule<NumericExpression *(Scope &)> num_var_ref;
	rule<ArithmeticOperation::Operator()> arith_operator;
	UnboundReferenceParser<NumericExpression> num_reference;
};




struct BooleanExpressionParser : grammar<BooleanExpression *(Scope &)> {
	BooleanExpressionParser() : BooleanExpressionParser::base_type(expression)
	{
		expression = atom(_r1) | formula(_r1);

		atom = bool_constant | bool_var_ref(_r1) | bool_reference(_r1);
		bool_var_ref = bool_var(_r1) [ _val = new_<Reference<BooleanVariable>>(_1, _r1) ];
		formula = operation(_r1) | num_comparison(_r1) | negation(_r1) | brace(_r1);

		operation = (
			(expression(_r1) >> bool_op) > expression(_r1)
		) [
			_val = new_<BooleanOperation>(at_c<0>(_1), at_c<1>(_1), _2, _r1)
		];

		num_comparison = (
			(num_expression(_r1) >> num_cmp_op) > num_expression(_r1)
		) [
			_val = new_<Comparison>(at_c<0>(_1), at_c<1>(_1), _2, _r1)
		];

		bool_op =
			qi::string("==") [ _val = val(BooleanOperator::IFF) ]
			| qi::string("!=") [ _val = val(BooleanOperator::XOR) ]
			| qi::string("&") [ _val = val(BooleanOperator::AND) ]
			| qi::string("|") [ _val = val(BooleanOperator::OR) ]
			| qi::string("->") [ _val = val(BooleanOperator::IMPLIES) ]
		;

		num_cmp_op =
			qi::string(">") [ _val = val(ComparisonOperator::GT) ]
			| qi::string(">=") [ _val = val(ComparisonOperator::GE) ]
			| qi::string("<=") [ _val = val(ComparisonOperator::LE) ]
			| qi::string("<") [ _val = val(ComparisonOperator::LT) ]
			| qi::string("==") [ _val = val(ComparisonOperator::NEQ) ]
			| qi::string("!=") [ _val = val(ComparisonOperator::EQ) ]
		;

		negation = '!' > expression(_r1) [
			_val = new_<Negation>(construct<unique_ptr<BooleanExpression>>(_1), _r1)
		];

		brace = '(' > expression(_r1) > ')';


	}

	rule<BooleanExpression *(Scope &)> expression;
	rule<BooleanExpression *(Scope &)> atom;
	rule<BooleanExpression *(Scope &)> formula;
	rule<BooleanExpression *(Scope &)> operation;
	rule<BooleanExpression *(Scope &)> negation;
	rule<BooleanExpression *(Scope &)> brace;
	rule<BooleanExpression *(Scope &)> bool_var_ref;
	UnboundReferenceParser<BooleanExpression> bool_reference;
	rule<BooleanOperator()> bool_op;
	rule<BooleanExpression *(Scope &)> num_comparison;
	NumericExpressionParser num_expression;
	rule<ComparisonOperator()> num_cmp_op;
};



struct StatementParser : grammar<Statement *(Scope &)> {
	StatementParser() : StatementParser::base_type(statement)
	{
		statement = choose(_r1) | conditional(_r1) | bool_assignment(_r1)
			| numeric_assignment(_r1) | pick(_r1) | search(_r1) | test(_r1) | r_while(_r1)
			| boolean_return(_r1) | numeric_return(_r1) | procedure_call(_r1) | block(_r1);

		block = ('{' > (statement(_r1) % ';') > '}') [
			_val = new_<Block>(_1, _r1)
		];

		choose = (l("choose") > '{' > (statement(_r1) % ',') > '}') [
			_val = new_<Choose>(_1, _r1)
		];

		conditional = (l("if") > '(' > boolean_expression(_r1) > ')'
			> statement(_r1) > -("else" > statement(_r1))
		) [
			_val = new_<Conditional>(_1, _2, _3, _r1)
		];

		bool_assignment = (bool_fluent_ref(_r1) >> '=' >> boolean_expression(_r1)) [
			_val = new_<Assignment<BooleanExpression>>(_1, _2, _r1)
		];

		numeric_assignment = (num_fluent_ref(_r1) >> '=' >> numeric_expression(_r1)) [
			_val = new_<Assignment<NumericExpression>>(_1, _2, _r1)
		];

		pick = (l("pick") > '(' > abstract_var(_r1) > ')' > statement(_r1)) [
			_val = new_<Pick>(_1, _2, _r1)
		];

		search = (l("search") > statement(_r1)) [
			_val = new_<Search>(_1, _r1)
		];

		test = (l("test") > '(' > boolean_expression(_r1) > ')') [
			_val = new_<Test>(_1, _r1)
		];

		r_while = (l("while") > '(' > boolean_expression(_r1) > ')' > statement(_r1)) [
			_val = new_<While>(_1, _2, _r1)
		];

		boolean_return = (l("return") > boolean_expression(_r1)) [
			_val = new_<Return<BooleanExpression>>(_1, _r1)
		];

		numeric_return = (l("return") > numeric_expression(_r1)) [
			_val = new_<Return<NumericExpression>>(_1, _r1)
		];
	}

	rule<Statement *(Scope &)> statement;
	rule<Block *(Scope &)> block;
	rule<Choose *(Scope &)> choose;
	rule<Conditional *(Scope &)> conditional;
	rule<Assignment<BooleanExpression> *(Scope &)> bool_assignment;
	rule<Assignment<NumericExpression> *(Scope &)> numeric_assignment;
	rule<Pick *(Scope &)> pick;
	rule<Search *(Scope &)> search;
	rule<Test *(Scope &)> test;
	rule<While *(Scope &)> r_while;
	rule<Return<BooleanExpression> *(Scope &)> boolean_return;
	rule<Return<NumericExpression> *(Scope &)> numeric_return;
	UnboundReferenceParser<Procedure> procedure_call;
	BooleanExpressionParser boolean_expression;
	NumericExpressionParser numeric_expression;
	UnboundReferenceParser<BooleanFluent> bool_fluent_ref;
	UnboundReferenceParser<NumericFluent> num_fluent_ref;
};



template<class ExpressionT>
struct FunctionParser : grammar<Function<ExpressionT> *()> {
	FunctionParser()
	: FunctionParser::base_type(function)
	, scope(nullptr)
	{
		function = (
			((type_mark<ExpressionT>() >> "function") > r_name > '(') [
				delete_(_r(scope)),
				_r(scope) = new_<Scope>(nullptr)
			]
			> *abstract_var(*_r(scope)) > ')' > statement(*_r(scope))
		) [
			_val = new_<Function<ExpressionT>>(_r(scope), _1, _2, _3)
		];
	}

	rule<Function<ExpressionT> *()> function;
	Scope *scope;
	StatementParser statement;
};



struct AbstractFunctionParser : grammar<AbstractFunction *()> {
	AbstractFunctionParser()
	: AbstractFunctionParser::base_type(function)
	{
		function = bool_func | num_func;
	}

	rule<AbstractFunction *()> function;
	FunctionParser<NumericExpression> num_func;
	FunctionParser<BooleanExpression> bool_func;
};




struct EffectParser : grammar<AbstractEffectAxiom *(Action &, Scope &)> {
	EffectParser()
	: EffectParser::base_type(effect)
	{
		effect = boolean_effect(_r1, _r2) | numeric_effect(_r1, _r2);

		boolean_effect = (
			(boolean_expression(_r2) >> "->"
			>> bool_fluent_ref(_r2)) > '=' > boolean_expression(_r2)
		) [
			// Bracketing above create a 2-tuple as first attribute, so use at_c
			// to access its elements.
			_val = new_<EffectAxiom<BooleanExpression>>(_r1, at_c<0>(_1), at_c<1>(_1), _2)
		];

		numeric_effect = (
			(boolean_expression(_r2) >> "->"
			>> num_fluent_ref(_r2)) > '=' > numeric_expression(_r2)
		) [
			_val = new_<EffectAxiom<NumericExpression>>(_r1, at_c<0>(_1), at_c<1>(_1), _2)
		];
	}

	rule<AbstractEffectAxiom *(Action &, Scope &)> effect;
	BooleanExpressionParser boolean_expression;
	NumericExpressionParser numeric_expression;
	UnboundReferenceParser<BooleanFluent> bool_fluent_ref;
	UnboundReferenceParser<NumericFluent> num_fluent_ref;
	rule<EffectAxiom<BooleanExpression> *(Action &, Scope &)> boolean_effect;
	rule<EffectAxiom<NumericExpression> *(Action &, Scope &)> numeric_effect;
};



struct ActionParser : grammar<Action *()> {
	ActionParser()
	: ActionParser::base_type(action)
	, scope(nullptr)
	{
		action = (
			(("action" > r_name > '(') [ delete_(_r(scope)), _r(scope) = new_<Scope>(nullptr) ])
			// CRUCIAL detail: use lazy dereference, i.e. *_r(scope), not _r(*scope).
			// Otherwise, we create a reference to the contents of an uninitialized pointer.
			> *abstract_var(*_r(scope)) > ')'
		) [
			_val = new_<Action>(_r(scope), _1, _2)
		]
		> -( '{'
			> -( "precondition:" > formula(*_r(scope)) [
				phoenix::bind(
					&Action::set_precondition,
					*_val,
					_1
				)
			] )
			> -( "effect:" > (effect(*_val, *_r(scope)) [
				phoenix::bind(
					&Action::add_effect,
					*_val,
					_1
				)
			] % ';') )
		//>> "signal:" //*/
		> '}' )
		;
	}

	BooleanExpressionParser formula;
	EffectParser effect;
	Scope *scope;
	rule<Action *()> action;
};



struct FluentParser : grammar<AbstractFluent *()> {
	FluentParser()
	: FluentParser::base_type(fluent)
	, scope(nullptr)
	{
		fluent = (
			(((type_mark<BooleanExpression>() >> "fluent") > r_name > '(') [ delete_(_r(scope)), _r(scope) = new_<Scope>(nullptr) ])
			> *abstract_var(*_r(scope)) > ')' > '=' > bool_constant
		) [
			_val = new_<BooleanFluent>(_r(scope), _1, _2, construct<unique_ptr<BooleanExpression>>(_3))
		]
		| (
			((type_mark<NumericExpression>() >> "fluent") > r_name > '(') [ delete_(_r(scope)), _r(scope) = new_<Scope>(nullptr) ]
			> *abstract_var(*_r(scope)) > ')' > '=' > num_constant
		) [
			_val = new_<NumericFluent>(_r(scope), _1, _2, construct<unique_ptr<NumericExpression>>(_3))
		];
	}

	rule<AbstractFluent *()> fluent;
	Scope *scope;
};



struct ProgramParser : grammar<Statement *(Scope &)> {
	ProgramParser()
	: ProgramParser::base_type(program)
	{
		program = *( omit[ // Discard attributes, just register as Globals
			fluent [
				phoenix::bind(&Scope::register_global, _r1, _1)
			]
			| action [
				phoenix::bind(&Scope::register_global, _r1, _1)
			]
			| function [
				phoenix::bind(&Scope::register_global, _r1, _1)
			]
		] ) > statement(_r1);
	}

	rule<Statement *(Scope &)> program;
	FluentParser fluent;
	ActionParser action;
	AbstractFunctionParser function;
	StatementParser statement;
};



} // namespace parser
} // namespace gologpp


#endif // GOLOGPP_GRAMMAR_H_
