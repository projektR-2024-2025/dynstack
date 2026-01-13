#include "SimulatorEvalOp.h"

bool SimulatorEvalOp::initialize(StateP state)
{
    if (!state->getGenotypes()[0]->isParameterDefined(state, "terminalset"))
        return false;

	voidP sptr = state->getGenotypes()[0]->getParameterValue(state, "terminalset");

    std::string terminals = *((std::string*)sptr.get());

    std::stringstream ss(terminals);
    std::string token;
    this->terminal_names_.clear();

    while (ss >> token) {
        this->terminal_names_.push_back(token);
    }

	return true;
}


FitnessP SimulatorEvalOp::evaluate(IndividualP individual)
{
    FitnessP fitness(new FitnessMin);

	fitness->setValue(0);

    return fitness;
}