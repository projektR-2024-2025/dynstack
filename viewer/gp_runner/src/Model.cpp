#include "Model.h"

bool TreeModel::initialize(StateP state) {
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

FitnessP TreeModel::evaluate(IndividualP individual) {
    FitnessP fitness(new FitnessMin);

	fitness->setValue(0);

    return fitness;
}

void TreeModel::execute(double& result, std::vector<double> features) {
    Tree::Tree* tree = (Tree::Tree*)this->genotype_.get();

    for (size_t i = 0; i < this->terminal_names_.size() && i < features.size(); ++i) {
        tree->setTerminalValue(this->terminal_names_[i], (void*)&features[i]);
    }

    tree->execute(&result);
}

FitnessP CGPModel::evaluate(IndividualP individual) {
    FitnessP fitness(new FitnessMin);

	fitness->setValue(0);

    return fitness;
}

void CGPModel::execute(double& result, std::vector<double> features) {
	Cartesian::Cartesian* cartesian = (Cartesian::Cartesian*)this->genotype_.get();

	std::vector<double> results;
	cartesian->evaluate(features, results);

	result = results[0];
}