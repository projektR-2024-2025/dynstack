#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stack>
#include <random>
#include <iostream>
#include <iomanip>

#include "Parameters.h"

#if defined(_WIN32) || defined(_WIN64)
    #define CLEAR_TERM "cls"  // Windows
#else
    #define CLEAR_TERM "clear" // Linux/macOS
#endif

struct Container {
    int id, wait, overdue, arrival_time;
    Container(int id, int w, int o, int t);
    bool is_ready(int current_time) const;
    bool is_overdue(int current_time) const;
    int get_overdue(int current_time) const;
};

struct World {
    int time;
    std::stack<Container> arrival_stack, handover_stack;
    std::stack<Container> buffers[3];
    const int max_arrival_size;
    const int max_buffer_size;
    int KPI[3];
};

class Simulator {
private:
    int time = 0;
    std::stack<Container> arrival_stack, handover_stack;
    std::stack<Container> buffers[3];
    std::mt19937 rng;
    inline static double seed = Parameters::SIMULATOR_SEED;
    int next_id = 1;
    int processed_count = 0;
    int KPI[3] = {0, 0, 0};
    bool is_crane_avail = true;

    void initalize_buffers();
    void generate_arrival();
    bool process_handover_top();

public:
    Simulator();
    ~Simulator() = default;

    World getWorld();
    bool move_arrival_to_buffer(int buffer_id);
    bool move_buffer_to_buffer(int from_buffer_id, int to_buffer_id);
    bool move_buffer_to_handover(int buffer_id);
    bool move_arrival_to_handover();
    void print_status();
    void print_state();
    void step();

    static void seed_simulator();
};

#endif // SIMULATOR_H