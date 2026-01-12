#include "Heuristic.h"
#include "Runner.h"
#include "Simulator.h"
#include "Parameters.h"


int main(int argc, char** argv) {

    Simulator sim;
    CustomHeuristic heuristic = CustomHeuristic() ;
	double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
    std::cout <<"KPI score of custom heuristic: " <<score <<std::endl ;
}