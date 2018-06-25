#ifndef GOLOGPP_ERROR_H_
#define GOLOGPP_ERROR_H_

#include "gologpp.h"

#include <exception>
#include <string>

namespace gologpp {

class InconsistentModel : public std::exception {
};

class EngineError : public std::exception {
};

class Bug : public std::exception {
public:
	Bug(const std::string &msg);
	virtual ~Bug() override = default;

	virtual const char *what() const noexcept override;

private:
	std::string msg_;
};


class UserError : public std::exception {
};

class InvalidExpressionType : public UserError {
public:
	InvalidExpressionType(const Expression &expr);

private:
	const Expression &expression;
};

}



#endif /* GOLOGPP_ERROR_H_ */
