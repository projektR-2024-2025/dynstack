#ifndef SimulatorEvalOp_h
#define SimulatorEvalOp_h
#include <ECF/ECF.h>

class SimulatorEvalOp : public EvaluateOp
{
public:
    FitnessP evaluate(IndividualP individual);
};
typedef std::shared_ptr<SimulatorEvalOp> SimulatorEvalOpP;


#endif // OneMaxEvalOp_h