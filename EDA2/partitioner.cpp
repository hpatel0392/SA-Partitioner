/*
* Harsh Patel
* Spring 2018
* Implementation of Partitioner class
*/

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#include <string>
#include <set>
#include "partitioner.h"

Partitioner::Partitioner(const Circuit & c) :
	keysA(),
	A(),
	keysB(),
	B(),
	sizeA( 0 ),
	sizeB( 0 ),
	cutSize( 0 )
{
	// reserve size for each partition (60% of total gates MAX)
	int MAX_CAP = (int)ceil(0.6 * (c.getInputs().size() + c.getGates().size() + c.getOutputs().size()) );
	keysA.reserve(MAX_CAP);
	keysB.reserve(MAX_CAP);

	// move inputs to A
	for (auto & in : c.getInputs()) {
		this->addToA(in);
	}

	//move outputs to B
	for (auto & out : c.getOutputs()) {
		this->addToB(out);
	}

	srand(time(0));

	// randomly distribute the remaining gates
	double thresh;
	double r;
	for (auto & g : c.getGates()) {
		thresh = ((double)sizeA / (sizeA + sizeB)) * 100.0;
		r = rand() % 100 + 1;
		if ( (r > thresh ) ) {
			this->addToA(g);
		} else {
			this->addToB(g);
		}
	}
	cutSize = calculateCutSize();
}

Partitioner::Partitioner(const Partitioner & p) :
	keysA(),
	A(),
	keysB(),
	B(),
	sizeA(0),
	sizeB(0),
	cutSize(0)
{
	this->sizeA = p.sizeA;
	this->sizeB = p.sizeB;
	this->cutSize = p.cutSize;

	for (std::string  s : p.keysA) {
		this->keysA.emplace_back(s);
	}
	for (std::string  s : p.keysB) {
		this->keysB.emplace_back(s);
	}

	for (auto & p : p.A) {
		this->A.emplace(p);
	}
	for (auto & p : p.B) {
		this->B.emplace(p);
	}
}

Partitioner::~Partitioner() {
	keysA.clear();
	keysB.clear();
	A.clear();
	B.clear();
}


/* --------------------------- Simulated Anealing Algorithm ------------------------------*/

void Partitioner::PartitionSA(std::ofstream & out) {
	double To, Ti, Tf;
	int moves_per_temp;
	double k;
	int move, a, b;
	int cost;
	double coolFactor;

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> uniform(0.0, 1.0);

	// lambda function for move acceptance
	auto & acceptMove = [this](double k, double Ti, int cost, double r, int move) {
		if (move == 0 && !this->canAddToB() ||
			move == 1 && !this->canAddToA()) return false;

		if (cost < 0) return true;
		else {
			double boltz = exp((-cost) / (k * Ti));
			if (r < boltz) return true;
		}
		return false;
	};

	// lambda function to determine cost of a particular move
	auto & getCost = [this](int move, int a, int b) {
		switch (move) {
		case 0:
			return this->costMoveAToB(a);
			break;
		case 1:
			return this->costMoveBToA(b);
			break;
		case 2:
			return this->costSwapAB(a, b);
			break;
		default:
			return 0;
			break;
		}
	};

	// lambda function to make the specified move
	auto & makeMove = [this](int move, int a, int b) {
		switch (move) {
		case 0:
			this->moveFromAToB(a);
			break;
		case 1:
			this->moveFromBToA(b);
			break;
		case 2:
			this->swapAB(a, b);
			break;
		default:
			break;
		}
	};

	/*   IGNORE - THESE ARE FOR PLOTS
	std::ofstream costF("cost.txt", std::ofstream::out | std::ofstream::trunc);
	std::ofstream tempF("temp.txt", std::ofstream::out | std::ofstream::trunc);
	std::ofstream acceptF("accept.txt", std::ofstream::out | std::ofstream::trunc);
	*/

	// Param init
	To = Ti = 40000.0;
	Tf = Ti / pow(10, (floor(log10(Ti) + 1)));
	int totalNodes = getNodesA() + getNodesB();
	coolFactor = 0.97 + (0.001 * log(totalNodes)) ;
	k = .00025;
	moves_per_temp = (int)ceil( log(totalNodes) * sqrt(totalNodes) );
	double alpha = 0.9995;

	double accepted = 0.0;
	while (Ti > Tf) {
		accepted = 0.0;
		for (int i = 0; i < moves_per_temp; i++) {
			a = (int)floor(uniform(mt) * this->getNodesA() );	// pick a node is A
			b = (int)floor(uniform(mt) * this->getNodesB() );	// pick a node in B
			move = (int)floor(uniform(mt) * 3);		// select a move
			cost = getCost(move, a, b);				// determine cost of that move
			if (acceptMove(k, Ti, cost, uniform(mt), move)) {  // if accepted
				makeMove(move, a, b);   //make that move
				accepted++;		// this is a counter for the plots
				cutSize += cost;  // update cutsize
			}
		}

		Ti *= coolFactor;  // cool down
		moves_per_temp = (int)((double)moves_per_temp / (alpha));

		/* IGNORE
		costF << this->cutSize << std::endl;
		tempF << Ti << std::endl;
		acceptF << Ti << " " << (accepted / moves_per_temp) << std::endl;
		*/
	}

	this->cutSize = calculateCutSize(); // this is used to verify cutSize (there was a bug somewhere this is the fix)

	/*costF.close(); IGNORE
	tempF.close();
	acceptF.close();*/
}

/* ----------------------------------- End of SA ------------------------------------------*/

void Partitioner::moveFromAToB(int i) {
	addToB(removeFromA(i));
}

void Partitioner::moveFromBToA(int i) {
	addToA(removeFromB(i));
}

void Partitioner::swapAB(int a, int b) {
	addToB(removeFromA(a));
	addToA(removeFromB(b));
}

int Partitioner::costMoveAToB(int i) const {
	Gate * g = A.find(keysA[i])->second;
	return costInB(g) - costInA(g);
}

int Partitioner::costMoveBToA(int i) const {
	Gate * g = B.find(keysB[i])->second;
	return costInA(g) - costInB(g);
}

int Partitioner::costSwapAB(int a, int b) const {
	Gate * gA = A.find(keysA[a])->second;
	Gate * gB = B.find(keysB[b])->second;
	int offset = 0;

	std::set<std::string> overlap;

	// check to avoid swap cost calculation errors
	for (Gate * out : gA->getFanOut()) {
		if (out == gB) {
			offset++;
			break;
		}
	}
	if (offset == 0) {
		for (Gate * out : gB->getFanOut()) {
			if (out == gA) {
				offset++;
				break;
			}
		}
	}

	for (Gate * inA : gA->getFanIn()) {
		for (Gate * inB : gB->getFanIn()) {
			if (inA->getId() == inB->getId()) {
				overlap.emplace(inA->getId());
			}
		}
	}

	offset += overlap.size();

	return (costInA(gB) + costInB(gA) + offset) - (costInA(gA) + costInB(gB));
}


void Partitioner::printStats(std::ofstream & out) const {
	out << "Cutsize: " << cutSize << std::endl;
	out << "Size of P1 is: " << sizeA << "; Size of P2 is: " << sizeB << std::endl;
	out << "Partition 1" << std::endl;
	for (auto & p : A) {
		out << p.first << std::endl;
	}
	out << "Partition 2" << std::endl;
	for (auto & p : B) {
		out << p.first << std::endl;
	}
	out << "End of Results" << std::endl;
}

// this function is used to correct a bug with cutSize calculation but it it too costly
// to call everytime so I only call it at the end
int Partitioner::verifyCutSize() const {
	return cutSize - calculateCutSize();
}

/* ------------------- private methods ------------------------------- */

int Partitioner::calculateCutSize() const {
	int cSize = 0;

	// check all hyper edges from A to B
	for (auto & p : A) {
		for (auto * g : p.second->getFanOut()) {
			if (B.find(g->getId()) != B.end()) {
				cSize++;
				break;
			}
		}
	}

	// check all hyper edges from B to A
	for (auto & p : B) {
		for (auto * g : p.second->getFanOut()) {
			if (A.find(g->getId()) != A.end()) {
				cSize++;
				break;
			}
		}
	}

	return cSize;
}

int Partitioner::costInA(Gate* g) const {
	int cost = 0;
	bool inc = true;
	for (Gate * in : g->getFanIn()) {		// check fan ins of current gate in A
		if (B.find(in->getId()) != B.end()) {	// find it in B
			inc = true;
			for (Gate * out : in->getFanOut()) {	// check its fanouts
				if (A.find(out->getId()) != A.end()) {	// if in A
					if (out->getId() != g->getId()) { // check if not original
						inc = false;
						break;
					}
				}
			} // end inner for
			if (inc) cost++;
		}
	} // end outer for

	for (Gate * out : g->getFanOut()) {
		if (B.find(out->getId()) != B.end()) {
			cost++;
			break;
		}
	}
	return cost;
}

int Partitioner::costInB(Gate* g) const {
	int cost = 0;
	bool inc = false;
	for (Gate * in : g->getFanIn()) {	// check fan ins of current gate in B
		if (A.find(in->getId()) != A.end()) {  // find it in A
			inc = true;
			for (Gate * out : in->getFanOut()) {	// check its fanouts
				if (B.find(out->getId()) != B.end()) {	// if in B
					if (out->getId() != g->getId()) { // check if not original
						inc = false;
						break;
					}
				}
			} // end inner for
			if (inc) cost++;
		}
	} // end outer for


	for (Gate * out : g->getFanOut()) {
		if (A.find(out->getId()) != A.end()) {
			cost++;
			break;
		}
	}
	return cost;
}

// add the provided pair to partition A
// user should ensure size <= capacity of each partition
void Partitioner::addToA(std::pair<std::string, Gate*> p) {
	keysA.emplace_back(p.first);
	A.emplace(p);
	if (p.second->getType() == Gate::IN || p.second->getType() == Gate::OUT) {
		sizeA++;
	} else {
		sizeA += p.second->getFanIn().size();
	}
}

void Partitioner::addToB(std::pair<std::string, Gate*> p){
	keysB.emplace_back(p.first);
	B.emplace(p);
	if (p.second->getType() == Gate::IN || p.second->getType() == Gate::OUT) {
		sizeB++;
	}
	else {
		sizeB += p.second->getFanIn().size();
	}
}

std::pair<std::string, Gate*> Partitioner::removeFromA(int i) {
	std::string toRemove = keysA[i];
	keysA[i] = keysA[keysA.size()-1];
	keysA.pop_back();
	std::unordered_map<std::string, Gate*>::iterator it = A.find(toRemove);
	std::pair<std::string, Gate*> p(it->first, it->second);

	if (p.second->getType() == Gate::IN || p.second->getType() == Gate::OUT) {
		sizeA--;
	}
	else {
		sizeA -= p.second->getFanIn().size();
	}
	A.erase(toRemove);
	return p;
}

std::pair<std::string, Gate*> Partitioner::removeFromB(int i) {
	std::string toRemove = keysB[i];
	keysB[i] = keysB[keysB.size() - 1];
	keysB.pop_back();
	std::unordered_map<std::string, Gate*>::iterator it = B.find(toRemove);
	std::pair<std::string, Gate*> p(it->first, it->second);

	if (p.second->getType() == Gate::IN || p.second->getType() == Gate::OUT) {
		sizeB--;
	} else {
		sizeB -= p.second->getFanIn().size();
	}

	B.erase(toRemove);
	return p;
}
