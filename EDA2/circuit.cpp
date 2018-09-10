#include <string>
#include "circuit.h"

Circuit::Circuit(std::ifstream & in) :
	inputs(),
	gates(),
	outputs()
{ this->parseFile(in); }


Circuit::~Circuit(){
	for (auto & g : inputs) {
		delete g.second;
	}
	for (auto & g : gates) {
		delete g.second;
	}
	for (auto & g : outputs) {
		delete g.second;
	}

	inputs.clear();
	gates.clear();
	outputs.clear();
}


/**
*	Private function, only called by constructor
*	Parse the provided file and load the circuit with gate data
*/
void Circuit::parseFile(std::ifstream & in){
	std::string line, type;
	std::size_t found1, found2;
	std::string str1, str2;
	Gate *g, *g2;
	std::unordered_map<std::string, Gate*>::iterator it;

	auto removeSpace = [](std::string & s) {
		// To keep track of non-space character count
		int count = 0;

		std::string str = s;
		// Traverse the given string. If current character
		// is not space, then place it at index 'count++'
		for (int i = 0; str[i]; i++) 
			if (str[i] != ' ' && str[i] != '=')
				str[count++] = str[i]; // here count is incremented		
		str[count] = '\0';

		s = str.data();
	};

	while (!in.eof()) {
		getline(in, line);
		if (line.length() != 0 && line[0] != '#') {
			if (line.find(Gate::IN) != std::string::npos) {	// primary input
				found1 = line.find('(');
				found2 = line.find(')', found1 + 1);
				str1 = line.substr(found1 + 1, found2 - found1 - 1);
				removeSpace(str1);

				std::string gateStr = "G_" + str1;
				std::string inStr = "PI_" + str1;
				it = gates.find(gateStr);
				if (it == gates.end()) { // gate not yet created
					g = new Gate(Gate::IN, inStr);
				} else {
					g = it->second;
					g->setType(Gate::IN);
					g->setId(inStr);
					gates.erase(it);
				}

				inputs.emplace(inStr, g);

			}
			else if (line.find(Gate::OUT) != std::string::npos) {	// primary output
				found1 = line.find('(');
				found2 = line.find(')', found1 + 1);
				str1 = line.substr(found1 + 1, found2 - found1 - 1);
				removeSpace(str1);

				std::string outStr = "PO_" + str1;
				outputs.emplace(outStr, new Gate(Gate::OUT, outStr));

			}
			else {												// gate
				found1 = line.find('=');
				str1 = line.substr(0, found1);
				removeSpace(str1);

				found2 = line.find('(', found1 + 1);
				type = line.substr(found1 + 1, found2 - found1 - 1);
				removeSpace(type);
			
				std::string gateStr = "G_" + str1;
				// check to see if gate already made previously
				it = gates.find(gateStr);
				if (it == gates.end()) {
					g = new Gate(type, gateStr);		// if not, make one
				}
				else {
					g = it->second;
					g->setType(type);		// if yes set its type
				}

				found1 = found2;
				found2 = line.find(',', found1 + 1);
				while (found2 != std::string::npos) {
					str2 = line.substr(found1 + 1, found2 - found1 - 1);
					removeSpace(str2);

					g2 = getInputGate(str2);
					g2->addOut(g); // add current gate to its input's output list
					g->addIn(g2);  // add input gate to current gate's input list

					found1 = found2;
					found2 = line.find(',', found1 + 1);
				}
				found2 = line.find(')', found1 + 1);
				str2 = line.substr(found1 + 1, found2 - found1 - 1);
				removeSpace(str2);


				g2 = getInputGate(str2);
				g2->addOut(g); // add current gate to its input's output list
				g->addIn(g2);  // add input gate to current gate's input list

				gates.emplace(gateStr, g);	// add gate to gates

			} // end inner if
			g = g2 = nullptr;
		} // end outer if
	} // end while

	// connect any gates that were discovered before primary output was found
	for (auto & out : outputs) {
		found1 = out.first.find('_');
		str1 = out.first.substr(found1);
		str1 = "G" + str1;
		//str1 = out.first;
		it = gates.find(str1);
		if (it != gates.end()) {
			g = it->second;
			g->addOut(out.second);
			out.second->addIn(g);
		}
	}
}


/**
*	Debug function: just prints the data stored in the circuit
*/
void Circuit::dumpData(std::ofstream & out){
	out << "\nInputs:\n-------------------------\n";
	for (auto & g : inputs) {
		out << g.first << ": ";
		out << g.second->getId() << " " << g.second->getFanOut().size() << " " << g.second->getFanIn().size() << std::endl;
	}

	out << "\nGates:\n-------------------------\n";
	for (auto & g : gates) {
		out << g.first << ": ";
		out << g.second->getId() << " " << g.second->getFanOut().size() << " " << g.second->getFanIn().size() << std::endl;
	}

	out << "\nOutputs:\n-------------------------\n";
	for (auto & g : outputs) {
		out << g.first << ": ";
		out << g.second->getId() << " " << g.second->getFanOut().size() << " " << g.second->getFanIn().size() << std::endl;
	}
}


/*
*	Helper function that returns a pointer to the gate with key = num
*/
Gate * Circuit::getInputGate(const std::string & s){
	Gate * g;
	std::string inStr = "PI_" + s;
	std::string gStr = "G_" + s;
	std::unordered_map<std::string, Gate*>::iterator it = inputs.find(inStr);
	if (it == inputs.end()) {
		it = gates.find(gStr);
		if (it == gates.end()) {   // gate not found
			g = new Gate();
			g->setId(gStr);
			gates.emplace(gStr, g);
		}
		else { g = it->second; } // gate found in gates
	}
	else { g = it->second; } // gate found in inputs

	return g;
}

