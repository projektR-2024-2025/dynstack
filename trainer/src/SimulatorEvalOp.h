#ifndef SIMULATOR_EVAL_OP_H
#define SIMULATOR_EVAL_OP_H
#include <ECF/ECF.h>
#include "Runner.h"
#include "Heuristic.h"

class SimulatorEvalOp : public EvaluateOp
{
public:
    FitnessP evaluate(IndividualP individual);
};
typedef std::shared_ptr<SimulatorEvalOp> SimulatorEvalOpP;

#endif // SIMULATOR_EVAL_OP_H