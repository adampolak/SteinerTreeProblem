#include "MA.h"

int main(int argc, char **argv){
	int N = 10;
	double pc = 0.9;
	double pm = 0.01;
	double finalTime = 30 * 60;
	MA ma(N, pc, pm, finalTime);
	srand(time(NULL));
	string file = string(argv[1]);
	SteinerTreeProblem STP(file);
	SteinerTree::SteinerTreeproblem = &STP;
	ma.run();
}
