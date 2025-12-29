#include <ECF/ECF.h>

//
// pomocni operator za ispis trenutno najboljeg rjesenja
//
class WriteBest : public Operator
{
private:
	StateP state_;
public:

	bool initialize(StateP state)
	{
		state_ = state;
		return true;
	}

	bool operate(StateP state)
	{
		ECF_LOG(state, 3, "Best in " + uint2str(state->getGenerationNo()));
		IndividualP bestInd = state->getPopulation()->getHof()->getBest().at(0);
		ECF_LOG(state, 3, bestInd->toString());

		return true;
	}
};