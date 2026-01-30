#include "Model.h"

bool TreeModel::initialize(StateP state) {
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

    return true;
}

FitnessP TreeModel::evaluate(IndividualP individual) {
    FitnessP fitness(new FitnessMin);

    if (Parameters::RUN_BEST) {
        fitness->setValue(0);
        return fitness;
    }

    //Tree::Tree* tree = (Tree::Tree*)individual->getGenotype().get();
    this->set_genotype(individual->getGenotype());

    PriorityHeuristic heuristic(shared_from_this());

    Simulator sim;

    double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
    fitness->setValue(score);

    return fitness;
}

// implementacija TreeModel klase
void TreeModel::execute(double& result, std::vector<double> features) {
	Tree::Tree* tree = (Tree::Tree*)this->genotype_.get();
	//std::cout << tree->toString() << std::endl; // za testiranje

	for (size_t i = 0; i < this->terminal_names_.size(); ++i) {
		tree->setTerminalValue(this->terminal_names_[i], (void*)&features[i]);
	}

	tree->execute(&result);
}

FitnessP CGPModel::evaluate(IndividualP individual)
{
    FitnessP fitness(new FitnessMin);

    if (Parameters::RUN_BEST) {
        fitness->setValue(0);
        return fitness;
    }

    this->set_genotype(individual->getGenotype());

    PriorityHeuristic heuristic(shared_from_this());

    Simulator sim;

    double score = run_simulation(sim, heuristic, Parameters::SIM_STEPS);
    fitness->setValue(score);

    return fitness;
}

// implementacija CGPModel klase (TODO)
void CGPModel::execute(double& result, std::vector<double> features) {
	Cartesian::Cartesian* cartesian = (Cartesian::Cartesian*)this->genotype_.get();
	//std::cout << tree->toString() << std::endl; // za testiranje
	
	std::vector<double> results;
	cartesian->evaluate(features, results);

	result = results[0];
}