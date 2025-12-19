#include <ECF/ECF.h>
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