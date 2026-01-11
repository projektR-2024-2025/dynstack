#ifndef SIMULATOR_EVAL_OP_H
#define SIMULATOR_EVAL_OP_H
#include <ECF/ECF.h>
#include "Runner.h"
#include "Heuristic.h"
#include "Parameters.h"

class SimulatorEvalOp : public EvaluateOp
{
protected:
    std::vector<std::string> terminal_names_;

public:
    FitnessP evaluate(IndividualP individual);
    bool initialize(StateP);
};
typedef std::shared_ptr<SimulatorEvalOp> SimulatorEvalOpP;

#endif // SIMULATOR_EVAL_OP_H