#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stack>
#include <random>
#include <iostream>

#include "../starterkits/cpp/src/heuristic.h"

struct Container {
    int id, wait, overdue, arrival_time;
    Container(int id, int w, int o, int t);
    bool is_ready(int current_time) const;
    bool is_overdue(int current_time) const;
    int until_ready(int current_time) const;
    int get_overdue(int current_time) const;
};

struct World {
    int time;
    std::stack<Container> arrival_stack, handover_stack;
    std::stack<Container> buffers[3];
    const int max_buffer_size;
    int KPI[3];
};

class BufferSimulator {
private:
    int arrival_density;
    int time = 0;
    std::stack<Container> arrival_stack, handover_stack;
    std::stack<Container> buffers[3];
    std::mt19937 rng;
    int next_id = 1;
    const int max_buffer_size = 8;
    int processed_count = 0;
    int KPI[3] = {0, 0, 0};
    bool isCraneAvail = true;

    void initalize_buffers();
    void generate_arrival();
    bool process_handover_top();

public:
    BufferSimulator(int arrival_density, bool initalize_buffers);
    ~BufferSimulator() = default;

    World getWorld();
    bool move_arrival_to_buffer(int buffer_id);
    bool move_buffer_to_buffer(int from_buffer_id, int to_buffer_id);
    bool move_buffer_to_handover(int buffer_id);
    void print_status();
    void step();
    static double run_simulation(BufferSimulator& sim, AbstractHeuristic& heuristic, int max_steps);
};

#endif // SIMULATOR_H