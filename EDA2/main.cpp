/*
* Harsh Patel
* Spring 2018
* Driver file
*/

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>
#include "gate.h"
#include "circuit.h"
#include "partitioner.h"

int main(int argc, char * argv[]) {
	if (argc != 2) {
		std::cout << "Not enough arguements!\nUsage: <exe> <file-name>" << std::endl;
		exit(1);
	}

	std::string fileName(argv[1]);
	fileName = fileName.substr(0, fileName.find('.'));
	std::ifstream in(fileName + ".in");
	if (!in.is_open()) {
		std::cout << "Error! file cannot be opened!\n";
		exit(1);
	}
	std::ofstream out(fileName + ".out", std::ofstream::out | std::ofstream::trunc);


	// parse the input file and create the circuit
	Circuit circuit(in);


	// Partition
	Partitioner P(circuit);
	P.PartitionSA(out);

	P.printStats(out);

	//close files
	in.close();
	out.close();

	return 0;
}
