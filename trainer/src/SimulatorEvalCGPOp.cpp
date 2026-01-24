#include "SimulatorEvalCGPOp.h"
#include "Model.h"

FitnessP SimulatorEvalCGPOp::evaluate(IndividualP individual)
{
    FitnessP fitness(new FitnessMin);

    if (Parameters::RUN_BEST) {
        fitness->setValue(0);
        return fitness;
    }

    GenotypeP genotype = individual->getGenotype();
	CGPModel* model = new CGPModel(genotype);

    PriorityHeuristic heuristic(model);

    Simulator sim;

    double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
    fitness->setValue(score);

    return fitness;
}