#include "SimulatorEvalOp.h"

FitnessP SimulatorEvalOp::evaluate(IndividualP individual)
{
    FitnessP fitness(new FitnessMin);

    Tree::Tree* tree = (Tree::Tree*)individual->getGenotype().get();

	std::vector<std::string> terminal_names_ = { "t1", "t2", "t3", "t4", "t5", "t6" };
	fitness->setValue(0);

    return fitness;
}