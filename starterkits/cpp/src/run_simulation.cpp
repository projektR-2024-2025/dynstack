#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <ECF/ECF.h>
#include <ECF/State.h>
#include <ECF/Individual.h>
#include <ECF/tree/Tree.h>
#include <ECF/tree/Primitive.h>

#include "run_simulation.h"
#include "simulator.h"
#include "heuristic.h"


double run_simulation(BufferSimulator& sim, AbstractHeuristic& heuristic, int max_steps) {
    for(int step = 0; step < max_steps; ++step) {
        Move m = heuristic.calculate_move(sim);
        apply_move(sim, m);
        sim.step();
    }
    World w = sim.getWorld();
    return w.KPI[0] + w.KPI[1] + w.KPI[2];
}
