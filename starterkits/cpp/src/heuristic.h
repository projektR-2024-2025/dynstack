#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "simulator.h"
#include <memory>
#include <vector>
#include <optional>
#include <iostream>
#include <ECF/tree/Tree.h>

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
    virtual Move calculate_move(BufferSimulator& sim) = 0;
};

class CustomHeuristic : public AbstractHeuristic {
public:
    Move calculate_move(BufferSimulator& sim) override;

};

class PriorityHeuristic: public AbstractHeuristic {
public:
    PriorityHeuristic(Tree::Tree* tree, std::vector<std::string>& terminal_names);
    Move calculate_move(BufferSimulator& sim) override;
private:
    Tree::Tree* priority_tree;
    std::vector<std::string> terminals;

    double evaluate_move(BufferSimulator& sim, Move& m);
    std::vector<double> extract_features(World& w);
};

std::vector<Move> possible_moves(BufferSimulator& sim);
bool apply_move(BufferSimulator& sim, Move& m);


#endif