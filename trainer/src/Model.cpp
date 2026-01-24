#include "Model.h"

// implementacija TreeModel klase
void TreeModel::execute(double& result, std::vector<double> features) {
	Tree::Tree* tree = (Tree::Tree*)this->genotype_.get();
	//std::cout << tree->toString() << std::endl; // za testiranje


	for (size_t i = 0; i < this->terminal_names_.size(); ++i) {
		tree->setTerminalValue(this->terminal_names_[i], (void*)&features[i]);
	}

	tree->execute(&result);
}

// implementacija CGPModel klase (TODO)
void CGPModel::execute(double& result, std::vector<double> features) {
	Cartesian::Cartesian* cartesian = (Cartesian::Cartesian*)this->genotype_.get();
	//std::cout << tree->toString() << std::endl; // za testiranje
	
	std::vector<double> results;
	cartesian->evaluate(features, results);

	result = results[0];
}