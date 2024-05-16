#include "MA.h"
#include <unistd.h>

int main(int argc, char **argv){
	// alarm(30 * 60 + 10);
	int N = 100;
	double pc = 1;
	double pm = 0.01;
	double finalTime = 30 * 60;
	MA ma(N, pc, pm, finalTime);
	srand(888);
	SteinerTreeProblem STP;
	SteinerTree::SteinerTreeproblem = &STP;
	ma.run();
}
