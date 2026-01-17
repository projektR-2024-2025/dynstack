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
    // ARRIVAL TO HANDOVER???
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
    static std::vector<Move> possible_moves(Simulator& sim) {
        std::vector<Move> moves;
        World w = sim.getWorld();

        if (!w.arrival_stack.empty()) {
            for (int b = 0; b < 3; ++b) {
                if (w.buffers[b].size() < w.max_buffer_size) {
                    moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
                }
            }
        }
        else {
            moves.push_back(Move{ MoveType::NONE, -1, -1 });
            // BUFFER -> HANDOVER
            for (int i = 0; i < 3; ++i) {
                if (!w.buffers[i].empty()) {

                    Container& top = w.buffers[i].top();
                    if (top.is_ready(w.time)) {
                        moves.push_back(Move{ MoveType::BUFFER_TO_HANDOVER, i, -1 });
                    }
                }
            }
            // BUFFER -> BUFFER
            for (int i = 0; i < 3; ++i) {
                if (!w.buffers[i].empty()) {
                    for (int j = 0; j < 3; ++j) {
                        if (i != j && w.buffers[j].size() < w.max_buffer_size) {
                            moves.push_back(Move{ MoveType::BUFFER_TO_BUFFER, i, j });
                        }
                    }
                }
            }
        }
        return moves;
    }
    static bool apply_move(Simulator& sim, Move& m) {
        switch (m.type) {
        case MoveType::ARRIVAL_TO_BUFFER: return sim.move_arrival_to_buffer(m.to);
        case MoveType::BUFFER_TO_BUFFER: return sim.move_buffer_to_buffer(m.from, m.to);
        case MoveType::BUFFER_TO_HANDOVER: return sim.move_buffer_to_handover(m.from);
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