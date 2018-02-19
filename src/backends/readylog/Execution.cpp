#include "Execution.h"

#include <eclipseclass.h>
#include "Action.h"
#include <gologpp/Action.h>

#include <iostream>

namespace gologpp {

EclipseContext::EclipseContext()
: ExecutionContext<readylog_impl_config_t>()
{
	ec_set_option_ptr(EC_OPTION_ECLIPSEDIR, (void *)ECLIPSE_DIR);
	std::cout << "Using eclipse-clp in " << ECLIPSE_DIR << std::endl;
	if (ec_init())
		throw std::runtime_error("Error initializing ECLiPSe subsystem");

	std::cout << "Loading readylog from " << READYLOG_PATH " ..." << std::endl;
	post_goal(::term(EC_functor("compile", 1), EC_atom(READYLOG_PATH)));
	EC_resume();
	std::cout << "... done." << std::endl;
}


EclipseContext::~EclipseContext()
{
	ec_cleanup();
}


EclipseContext &EclipseContext::instance()
{
	static EclipseContext ctx;
	return ctx;
}


void EclipseContext::compile(const EC_word &term)
{
	post_goal(::term(EC_functor("assert", 1), term));
	post_goal(EC_atom("listing"));
	int rv = EC_resume();
	if (rv != EC_status::EC_succeed)
		throw EclipseError();
}



} // namespace gologpp
