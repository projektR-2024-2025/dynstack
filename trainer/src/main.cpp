#include <ECF/ECF.h>
#include "WriteBest.h"
#include "Parameters.h"
#include "SeedSimulator.h"
#include "Heuristic.h"
#include "Runner.h"
#include "Model.h"

int main(int argc, char** argv) {

    Parameters::readParameters(argc, argv);

    if (Parameters::SEED_SIM)
        Simulator::seed_simulator();

    if (Parameters::USING_ECF) {
        StateP state(new State);
        ModelP model;

        switch(Parameters::MODEL) {
            // za GP
            case 0:
                model = std::make_shared<TreeModel>(); break;
            // za CGP
            case 1:
                model = std::make_shared<CGPModel>(); break;
            default:
                cout << "The given genotype is not supported." << endl;
                return 2;
        }

        state->setEvalOp(model);
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
            Simulator sim;
            model->set_genotype((GenotypeP) ind->getGenotype().get());
            PriorityHeuristic heuristic(model);
            std::cout << "Running best individual" << std::endl;
            double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
            std::cout << "KPI score of the best individual: " << score << std::endl;
        }
    }
    else {
        std::cout << "Running custom heuristic" << std::endl;
        Simulator sim;

        CustomHeuristic heuristic;
        double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
        std::cout << "KPI score of custom heuristic: " << score << std::endl;
    }

    return 0;
}