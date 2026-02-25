#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stack>
#include <queue>
#include <random>
#include <iostream>
#include <iomanip>
#include <vector>

#include "Parameters.h"

#define KPIs 8

enum class MoveType;

struct Container {
    int id, wait, overdue, arrival_time;
    Container(int id, int w, int o, int t) : id(id), wait(w), overdue(o), arrival_time(t) {}
    bool is_ready(int current_time) const {
        return (current_time - arrival_time) > wait;
    }
    bool is_overdue(int current_time) const {
        return (current_time - arrival_time) > (overdue + wait);
    }
    int get_overdue(int current_time) const {
        return (current_time - arrival_time) - (overdue + wait);
    }
};

struct KPI_t {
    int blocked_arrival;
    int blocks_on_time;
    int crane_manipulations;
    int delivered_blocks;
    double leadtime;
    double service_level;
    double buffer_util;
    double handover_util;

    void print() {
        std::cout << "Blocked arrival: " << blocked_arrival << ", Blocks on time: " << blocks_on_time
            << ", Crane manipulations: " << crane_manipulations << ", Delivered blocks: " << delivered_blocks
            << ", Leadtime: " << leadtime << ", Service level: " << service_level << ", Buffer util: " << buffer_util
            << ", Handover util: " << handover_util << std::endl;
    }
};

struct World {
    int time;
    std::queue<Container> arrival_stack;
    std::stack<Container> handover_stack;
    std::vector<std::stack<Container>> buffers;
    const int max_arrival_size;
    const int max_buffer_size;
    KPI_t KPI;
};

class Simulator {
private:
    int time = 0;
    std::queue<Container> arrival_stack;
    std::stack<Container> handover_stack;
    std::vector<std::stack<Container>> buffers;
    std::mt19937 rng;
    inline static double seed = Parameters::SIMULATOR_SEED;
    int next_id = 1;
    KPI_t KPI = {0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0};
    int delivered_on_time = 0;
    int leadtime = 0;
    bool is_crane_avail = true;
    int made_move = 0;
    MoveType last_move;

    void initalize_buffers();
    void generate_arrival();
    bool process_handover_top();
    void calculate_KPI();

public:
    Simulator();
    ~Simulator() = default;

    World getWorld();
    bool getCraneState() { return is_crane_avail; }
    MoveType getLastMove() { return last_move; }
    bool move_arrival_to_buffer(int buffer_id);
    bool move_buffer_to_buffer(int from_buffer_id, int to_buffer_id);
    bool move_buffer_to_handover(int buffer_id);
    bool move_arrival_to_handover();
    bool no_move();
    void print_status();
    void print_state();
    void step();

    static void seed_simulator();
};

#endif // SIMULATOR_H