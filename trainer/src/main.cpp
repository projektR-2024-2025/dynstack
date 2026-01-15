#include <ECF/ECF.h>
#include "SimulatorEvalOp.h"
#include "WriteBest.h"
#include "Parameters.h"
#include "SeedSimulator.h"
#include "Heuristic.h"
#include "Runner.h"

int main(int argc, char** argv) {

    Parameters::readParameters(argc, argv);

    if (Parameters::USING_ECF) {
        StateP state(new State);

        SimulatorEvalOp evalOp;

        state->setEvalOp(&evalOp);
        state->addOperator((OperatorP) new WriteBest);
        state->addOperator((OperatorP) new SeedSimulator);
        state->initialize(argc, argv);

        if (!Parameters::RUN_BEST) {
            state->run();

            std::vector<IndividualP> hof = state->getHoF()->getBest();
            IndividualP ind = hof[0];
            std::ofstream best(Parameters::BEST_FILE);
            best << ind->toString();
            best.close();
        }
        else {
            XMLNode xInd = XMLNode::parseFile(Parameters::BEST_FILE.c_str(), "Individual");
            if (xInd.isEmpty()) {
                std::cout << "Can't run best individual because the file " << Parameters::BEST_FILE << " doesn't exist or isn't properly formated!" << std::endl;
                std::cout << "Exiting..." << std::endl;
                return 1;
            }
            IndividualP ind = (IndividualP) new Individual(state);
            ind->read(xInd);
            Tree::Tree* tree = (Tree::Tree*)ind->getGenotype().get();
            Simulator sim;

            PriorityHeuristic heuristic(tree, evalOp.terminal_names_);
            std::cout << "Running best individual" << std::endl;
            double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
            std::cout << "KPI score of the best individual: " << score << std::endl;
        }
    }
    else {
        std::cout << "Running custom heuristic" << std::endl;
        if (Parameters::SEED_SIM)
            Simulator::seed_simulator();
        Simulator sim;

        CustomHeuristic heuristic = CustomHeuristic();
        double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
        std::cout << "KPI score of custom heuristic: " << score << std::endl;
    }

    return 0;
}