#ifndef PARTITIONER_H
#define PARTITIONER_H

//#include <unordered_map>
#include <unordered_map>
#include <string>
#include "circuit.h"
#include "gate.h"

#define MAX_SIZE 0.56

class Partitioner {
public:
	Partitioner(const Circuit &);
	Partitioner(const Partitioner &);
	~Partitioner();

	int getNodesA() const { return A.size(); }
	int getNodesB() const { return B.size(); }
	int getSizeA() const { return sizeA; }
	int getSizeB() const { return sizeB; }
	int getCutSize() const { return cutSize; }
	bool canAddToA() const { return sizeA <= MAX_SIZE * (sizeB + sizeA); }
	bool canAddToB() const { return sizeB <= MAX_SIZE * (sizeB + sizeA); }

	void PartitionSA(std::ofstream &);

	int costMoveAToB(int) const ;
	int costMoveBToA(int) const ;
	int costSwapAB(int, int) const ;
	void moveFromAToB(int);
	void moveFromBToA(int);
	void swapAB(int, int);

	void printStats(std::ofstream &) const;
	int verifyCutSize() const;

private:

	std::vector<std::string> keysA;
	std::unordered_map<std::string, Gate *> A;
	std::vector<std::string> keysB;
	std::unordered_map<std::string, Gate *> B;

	int sizeA;
	int sizeB;
	int cutSize;
	
	int calculateCutSize() const;
	int costInA(Gate*) const ;
	int costInB(Gate*) const ;
	void addToA(std::pair<std::string, Gate*>);
	void addToB(std::pair<std::string, Gate*>);
	std::pair<std::string, Gate*> removeFromA(int);
	std::pair<std::string, Gate*> removeFromB(int);

	Partitioner() = delete;
	Partitioner(std::vector<std::string>, std::unordered_map<std::string, Gate*>, 
				std::vector<std::string>, std::unordered_map<std::string, Gate*>,
				int, int, int) = delete;
};

#endif
