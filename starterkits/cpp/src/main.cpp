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
#include "SimulatorEvalOp.h"

int main(int argc, char** argv) {
    StateP state(new State);

    state->setEvalOp(new SimulatorEvalOp);
    state->initialize(argc, argv);
    state->run();

    std::vector<IndividualP> hof = state->getHoF()->getBest();
    IndividualP ind = hof[0];
    std::ofstream best("./best.txt");
    best << ind->toString();
    best.close();
    return 0;
}