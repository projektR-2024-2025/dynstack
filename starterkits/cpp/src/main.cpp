#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <map>

#include <ECF/ECF.h>
#include <ECF/State.h>       // For StateP
#include <ECF/Individual.h>  // For IndividualP
#include <ECF/tree/Tree.h>
#include <ECF/tree/Tree_c.h>
#include <ECF/tree/Primitive.h>

#include "simulator.h"
#include "heuristic.h"

int main(int argc, char** argv) {

    StateP state(new State);
    state->initialize(argc, argv);
    auto pop = state->getPopulation();
    auto temp = (*pop)[0];
    IndividualP ind = (*temp)[0];
    
    Tree::Tree* tree = static_cast<Tree::Tree*>( ind->getGenotype(0).get() );

    std::vector<std::string> terminal_names = {"t1", "t2", "t3", "t4", "t5", "t6"};
    
    PriorityHeuristic heuristic(tree, terminal_names);
    BufferSimulator sim(2, true);
    double score = run_simulation(sim, heuristic, 50);
    
    std::cout << "Simulation KPI sum: " << score << std::endl;
    
    return 0;
}