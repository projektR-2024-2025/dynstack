#ifndef RUNNER_H
#define RUNNER_H

#include <chrono>
#include <thread>

#include "Simulator.h"
#include "Heuristic.h"
#include "Parameters.h"

static double run_simulation(Simulator& sim, AbstractHeuristic& heuristic, int max_steps) {
    for (int step = 0; step < max_steps; ++step) {
        Move m = heuristic.calculate_move(sim);
        AbstractHeuristic::apply_move(sim, m);
        if (!Parameters::USING_ECF || Parameters::RUN_BEST) {
            sim.print_state();
            m.print_move();
            sim.print_status();
            std::this_thread::sleep_for(std::chrono::milliseconds(Parameters::STEP_DURATION));
        }
        sim.step();
    }
    World w = sim.getWorld();

    double weights[KPIs] = {};
    std::istringstream ss(Parameters::KPI_WEIGHTS);
    double weight;
    int curr = 0;
    while (ss >> weight){
        weights[curr] = weight;
        curr++;
        if (curr >= KPIs)
            break;
    }
    return weights[0] * w.KPI.blocked_arrival + weights[1] * w.KPI.blocks_on_time + weights[2] * w.KPI.crane_manipulations + weights[3] * w.KPI.delivered_blocks +
        weights[4] * w.KPI.leadtime + weights[5] * w.KPI.service_level + weights[6] * w.KPI.buffer_util + weights[7] * w.KPI.handover_util;
}

#endif