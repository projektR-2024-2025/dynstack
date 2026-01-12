#include <ECF/ECF.h>
#include "SimulatorEvalOp.h"
#include "WriteBest.h"
#include "Parameters.h"
#include "SeedSimulator.h"
#include "Heuristic.h"
#include "Runner.h"

int main(int argc, char** argv) {

    Parameters::readParameters(argv[1]);

    if (Parameters::USING_ECF) {
        StateP state(new State);

        state->setEvalOp(new SimulatorEvalOp);
        state->addOperator((OperatorP) new WriteBest);
        state->addOperator((OperatorP) new SeedSimulator);
        state->initialize(argc, argv);
        state->run();

        std::vector<IndividualP> hof = state->getHoF()->getBest();
        IndividualP ind = hof[0];
        std::ofstream best(Parameters::BEST_FILE);
        best << ind->toString();
        best.close();
    }
    else {
        Simulator::seed_simulator();
        Simulator sim;

        CustomHeuristic heuristic = CustomHeuristic();
        double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
        std::cout << "KPI score of custom heuristic: " << score << std::endl ;
    }

    return 0;
}