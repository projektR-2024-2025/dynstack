#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "Simulator.h"

#include <vector>
#include <string>
#include <ECF/ECF.h>

namespace Tree {
    class Tree;
}

enum class MoveType {
    NONE,
    ARRIVAL_TO_BUFFER,
    BUFFER_TO_BUFFER,
    BUFFER_TO_HANDOVER
};

struct Move {
    MoveType type = MoveType::NONE;
    int from = -1;
    int to = -1;
};

class AbstractHeuristic {
public:
    virtual ~AbstractHeuristic() = default;
    virtual Move calculate_move(Simulator& sim) = 0;
    static std::vector<Move> possible_moves(Simulator& sim);
    static bool apply_move(Simulator& sim, Move& m);
};

class CustomHeuristic : public AbstractHeuristic {
public:
    Move calculate_move(Simulator& sim) override;
    
private:
    long long TUD(const Container& c, World& w);
    bool has_free_space(World& w, int buffer_id);
    int buffer_with_least_ready(World& w);
    int best_ready_on_top(World& w);
    struct CoveredReady {
        int buffer_id;
        int blocking_depth;
    };
    bool covered_ready_block(World& w, CoveredReady& out);
    int buffer_without_ready_on_top(World& w, int forbidden_buffer);
};

class PriorityHeuristic : public AbstractHeuristic {
public:
    PriorityHeuristic(Tree::Tree* tree, std::vector<std::string>& terminal_names);
    Move calculate_move(Simulator& sim) override;
private:
    Tree::Tree* priority_tree;
    std::vector<std::string> terminals;

    double evaluate_move(Simulator& sim, Move& m);
    std::vector<double> extract_features(World& w);
};

#endif