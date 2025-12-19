#ifndef RUNNER_H
#define RUNNER_H
#include "simulator.h"
#include "../starterkits/cpp/src/heuristic.h"

double run_simulation(BufferSimulator& sim, AbstractHeuristic& heuristic, int max_steps) {
    for (int step = 0; step < max_steps; ++step) {
        Move m = heuristic.calculate_move(sim);
        AbstractHeuristic::apply_move(sim, m);
        sim.step();
    }
    World w = sim.getWorld();
    return 0.5 * w.KPI[0] - 0.4 * w.KPI[1] + 0.1 * w.KPI[2];
}

#endif