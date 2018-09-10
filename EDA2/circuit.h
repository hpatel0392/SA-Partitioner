#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <fstream>
#include <istream>
#include <string>
#include <unordered_map>
#include "gate.h"

class Circuit {
public:
	Circuit(std::ifstream &);
	~Circuit();

	void dumpData(std::ofstream &);
	Gate * getInputGate(const std::string &);
	const std::unordered_map<std::string, Gate*> & getInputs() const { return inputs; }
	const std::unordered_map<std::string, Gate*> & getGates() const { return gates; }
	const std::unordered_map<std::string, Gate*> & getOutputs() const { return outputs; }

private:
	std::unordered_map<std::string, Gate*> inputs;
	std::unordered_map<std::string, Gate*> gates;
	std::unordered_map<std::string, Gate*> outputs;

	void parseFile(std::ifstream &);

	Circuit() = delete;
	Circuit(const Circuit&) = delete;

};
#endif
