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
    BUFFER_TO_HANDOVER,
    ARRIVAL_TO_HANDOVER
};

struct Move {
    MoveType type = MoveType::NONE;
    int from = -1;
    int to = -1;
    void print_move() {
        if (type == MoveType::ARRIVAL_TO_BUFFER)
            std::cout << std::left << std::string(15 * (to + 1), '-') << ">" << std::endl;
        else if (type == MoveType::BUFFER_TO_BUFFER) {
            if (from < to)
                std::cout << std::left << std::string(15 * (from + 1), ' ') << std::string(15 * (to - from), '-') << ">" << std::endl;
            else
                std::cout << std::left << std::string(15 * (to + 1), ' ') << "<" << std::string(15 * (from - to), '-') << std::endl;
        }
        else if (type == MoveType::BUFFER_TO_HANDOVER)
            std::cout << std::left << std::string(15 * (from + 1), ' ') << std::string(15 * (3 - from), '-') << ">" << std::endl;
        else if (type == MoveType::ARRIVAL_TO_HANDOVER)
            std::cout << std::left << std::string(15 * 4, '-') << ">" << std::endl;
        else
            std::cout << std::endl;
    }
};

class AbstractHeuristic {
public:
    virtual ~AbstractHeuristic() = default;
    virtual Move calculate_move(Simulator& sim) = 0;
    static bool apply_move(Simulator& sim, Move& m) {
        switch (m.type) {
        case MoveType::ARRIVAL_TO_BUFFER: return sim.move_arrival_to_buffer(m.to);
        case MoveType::BUFFER_TO_BUFFER: return sim.move_buffer_to_buffer(m.from, m.to);
        case MoveType::BUFFER_TO_HANDOVER: return sim.move_buffer_to_handover(m.from);
        case MoveType::ARRIVAL_TO_HANDOVER: return sim.move_arrival_to_handover();
        case MoveType::NONE: default: return false;
        }
    }
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
    static std::vector<Move> possible_moves(Simulator& sim);
    static std::vector<Move> priority_possible_moves(Simulator& sim, int MetaAlgParam = 0);
private:
    Tree::Tree* priority_tree;
    std::vector<std::string> terminals;

    double evaluate_move(Simulator& sim, Move& m);
    std::vector<double> extract_features(const World& before, const World& after, Move& m);

    // Interne funkcije kandidata meta-algoritama.

    static std::vector<Move> meta_alg_1(Simulator& sim);
    static std::vector<Move> meta_alg_2(Simulator& sim);
    static std::vector<Move> meta_alg_3(Simulator& sim);
};

#endif