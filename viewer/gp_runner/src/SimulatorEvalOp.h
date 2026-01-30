#ifndef SIMULATOR_EVAL_OP_H
#define SIMULATOR_EVAL_OP_H
#include <ECF/ECF.h>

class SimulatorEvalOp : public EvaluateOp
{
public:
    std::vector<std::string> terminal_names_;
    FitnessP evaluate(IndividualP individual);
    bool initialize(StateP);
};
typedef std::shared_ptr<SimulatorEvalOp> SimulatorEvalOpP;

#endif // SIMULATOR_EVAL_OP_H