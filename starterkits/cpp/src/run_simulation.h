#ifndef RUN_SIMULATION_H
#define RUN_SIMULATION_H

#include "simulator.h"
#include "heuristic.h"
//#include "sim_utils.h"

double run_simulation(BufferSimulator& sim, AbstractHeuristic& heuristic, int max_steps);

#endif
