#ifndef SIMULATOR_EVAL_CGP_OP_H
#define SIMULATOR_EVAL_CGP_OP_H
#include <ECF/ECF.h>
#include "Runner.h"
#include "Heuristic.h"
#include "Parameters.h"

class SimulatorEvalCGPOp : public EvaluateOp
{
public:
    FitnessP evaluate(IndividualP individual);
};
typedef std::shared_ptr<SimulatorEvalCGPOp> SimulatorEvalCGPOpP;

#endif