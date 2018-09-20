/*
* Harsh Patel
* Spring 2018]
* Definition of Gate Class
*/

#ifndef GATE_H
#define GATE_H

#include <cstdlib>
#include <vector>
#include <string>

class Gate {
public:
	static const std::string OUT;
	static const std::string IN;

	Gate();
	Gate(const std::string &, const std::string &);

	void setType(const std::string &);
	void setId(const std::string &);
	void addIn(Gate*);
	void addOut(Gate*);
	const std::string & getId() const { return id; }
	const std::string & getType() const { return type; }
	const std::vector<Gate *> & getFanIn() const { return in; }
	const std::vector<Gate *> & getFanOut() const { return out; }

private:
	std::string id;
	std::string type;
	std::vector<Gate*> in;
	std::vector<Gate*> out;

	Gate(const Gate&) = delete;
};

#endif
