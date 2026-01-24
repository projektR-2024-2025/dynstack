#ifndef MODEL_H
#define MODEL_H
#include <ECF/ECF.h>

class Model {
protected:
    // clanske var
    GenotypeP genotype_;

public:
    // konstruktori
    Model(GenotypeP genotype) : genotype_(genotype) {}
    //~Model();

    // metode
    virtual void execute(double& result, std::vector<double> features) = 0; // abstraktna, hence = 0
};

class TreeModel : public Model {
private:
    // clanske var
    std::vector<std::string> terminal_names_;

public:
    // konstruktori
    TreeModel(GenotypeP genotype, const std::vector<std::string> terminal_names)
        : Model(genotype), terminal_names_(terminal_names) {}
	//~TreeModel();

    // metode
	void execute(double& result, std::vector<double> features) override;
};

// TODO: CGP
class CGPModel : public Model {
public:
    // konstruktori
    CGPModel(GenotypeP genotype)
        : Model(genotype) {
    }
    //~CGPModel();

    // metode
    void execute(double& result, std::vector<double> features) override;
};


#endif