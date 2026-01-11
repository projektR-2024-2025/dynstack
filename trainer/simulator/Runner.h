#ifndef RUNNER_H
#define RUNNER_H
#include "Simulator.h"
#include "Heuristic.h"
#include "Parameters.h"

static double run_simulation(Simulator& sim, AbstractHeuristic& heuristic, int max_steps) {
    for (int step = 0; step < max_steps; ++step) {
        Move m = heuristic.calculate_move(sim);
        AbstractHeuristic::apply_move(sim, m);
        sim.step();
    }
    World w = sim.getWorld();
    return Parameters::KPI_W1 * w.KPI[0] + Parameters::KPI_W2 * w.KPI[1] + Parameters::KPI_W3 * w.KPI[2];
}

#endif