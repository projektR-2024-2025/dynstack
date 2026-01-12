#ifndef SEED_SIMULATOR_H
#define SEED_SIMULATOR_H

#include "Simulator.h"
#include <ECF/ECF.h>

class SeedSimulator : public Operator
{
private:
	StateP state_;
public:

	bool initialize(StateP state) {
		state_ = state;
		return true;
	}

	bool operate(StateP state) {
		Simulator::seed_simulator();

		return true;
	}
};

#endif