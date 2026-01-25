#ifndef MODEL_H
#define MODEL_H
#include <ECF/ECF.h>
#include "Runner.h"
#include "Heuristic.h"
#include "Parameters.h"

class Model : public EvaluateOp, public std::enable_shared_from_this<Model> {
protected:
    // clanske var
    GenotypeP genotype_;

public:
    // konstruktori
    Model() {}
    Model(GenotypeP genotype) : genotype_(genotype) {}
    virtual ~Model() = default;

    // metode
    virtual void execute(double& result, std::vector<double> features) = 0; // abstraktna, hence = 0
    void set_genotype(GenotypeP genotype) { genotype_ = genotype; }
};
typedef std::shared_ptr<Model> ModelP;

// GP
class TreeModel : public Model {
private:
    std::vector<std::string> terminal_names_;
public:
    // konstruktori
    TreeModel() : Model() {}
    TreeModel(GenotypeP genotype, const std::vector<std::string> terminal_names)
        : Model(genotype), terminal_names_(terminal_names) {}

    // metode
	void execute(double& result, std::vector<double> features);
    FitnessP evaluate(IndividualP individual);
    bool initialize(StateP);
};
typedef std::shared_ptr<TreeModel> TreeModelP;

// CGP
class CGPModel : public Model {
public:
    // konstruktori
    CGPModel() : Model() {}
    CGPModel(GenotypeP genotype) : Model(genotype) {}

    // metode
    void execute(double& result, std::vector<double> features);
    FitnessP evaluate(IndividualP individual);
};
typedef std::shared_ptr<CGPModel> CGPModelP;

#endif