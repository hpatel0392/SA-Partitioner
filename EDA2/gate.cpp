/*
* Harsh Patel
* Spring 2018
* Implementation of Gate class
*/

#include "gate.h"
#include <ostream>

const std::string Gate::OUT = "OUTPUT";
const std::string Gate::IN = "INPUT";

Gate::Gate() :
	id(),
	type(),
	in(),
	out()
{}

Gate::Gate(const std::string & s, const std::string & i) :
	id(i),
	type(s),
	in(),
	out()
{}

void Gate::setType(const std::string & t){
	type = t;
}

void Gate::setId(const std::string & i) {
	id = i;
}

void Gate::addIn(Gate * g){
	in.emplace_back(g);
}

void Gate::addOut(Gate * g){
	out.emplace_back(g);
}

std::ostream & operator<<(std::ostream & out, const Gate & g) {
	return out << g.getType();
}
