#include "SimulatorEvalOp.h"

// s ovom funkcijom dodjeljujem vrijednost clanskoj varijable klase SimulatorEvalOp
// (built-in ECF-ova funkcija)
bool SimulatorEvalOp::initialize(StateP state)
{
    if (!state->getGenotypes()[0]->isParameterDefined(state, "terminalset")) // ovo sam ostavio za provjeru je li definirano
        return false;

	voidP sptr = state->getGenotypes()[0]->getParameterValue(state, "terminalset"); // ovo ga fetcha

    // ovo nadalje je parsiranje
    std::string terminals = *((std::string*)sptr.get());

    std::stringstream ss(terminals);
    std::string token;
    this->terminal_names_.clear();

    while (ss >> token) {
        this->terminal_names_.push_back(token);
    }

    Simulator::seed_simulator();

	return true;
}

FitnessP SimulatorEvalOp::evaluate(IndividualP individual)
{
    FitnessP fitness(new FitnessMin);

    Tree::Tree* tree = (Tree::Tree*)individual->getGenotype().get();

	PriorityHeuristic heuristic(tree, this->terminal_names_);

	Simulator sim;

	double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
	fitness->setValue(score);

    return fitness;
}