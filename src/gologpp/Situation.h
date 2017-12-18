#ifndef GOLOGPP_SITUATION_H
#define GOLOGPP_SITUATION_H

#include "Execution.h"
#include "Fluent.h"

#include <unordered_map>

namespace gologpp {

using namespace std;

class Situation : public LanguageElement {
public:
	Situation();

protected:
	unordered_map<shared_ptr<Fluent>, unique_ptr<Value>> fluent_values_;
};


}

#endif // GOLOGPP_SITUATION_H