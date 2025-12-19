#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include <ECF/ECF.h>
#include <ECF/State.h>
#include <ECF/Individual.h>
#include <ECF/tree/Tree.h>
#include <ECF/tree/Primitive.h>

#include "heuristic.h"
#include <limits>

bool apply_move(BufferSimulator& sim, Move& m) {
    switch(m.type) {
        case MoveType::ARRIVAL_TO_BUFFER: return sim.move_arrival_to_buffer(m.to);
        case MoveType::BUFFER_TO_BUFFER: return sim.move_buffer_to_buffer(m.from, m.to);
        case MoveType::BUFFER_TO_HANDOVER: return sim.move_buffer_to_handover(m.from);
        case MoveType::NONE: default: return false;
    }
}

std::vector<Move> possible_moves(BufferSimulator& sim) {
    std::vector<Move> moves;
    World w = sim.getWorld();

    if (!w.arrival_stack.empty()) {
        for (int b = 0; b < 3; ++b) {
            if (w.buffers[b].size() < w.max_buffer_size) {
                moves.push_back(Move{MoveType::ARRIVAL_TO_BUFFER, -1, b});
            }
        }
    } else {
        moves.push_back(Move{MoveType::NONE, -1, -1});
        // BUFFER -> HANDOVER
        for (int i = 0; i < 3; ++i) {
            if (!w.buffers[i].empty()) {
                
                Container& top = w.buffers[i].top();
                if (top.is_ready(w.time)) {
                    moves.push_back(Move{MoveType::BUFFER_TO_HANDOVER, i, -1});
                }
            }
        }
        // BUFFER -> BUFFER
        for (int i = 0; i < 3; ++i) {
            if (!w.buffers[i].empty()) {
                for (int j = 0; j < 3; ++j) {
                    if (i != j && w.buffers[j].size() < w.max_buffer_size) {
                        moves.push_back(Move{MoveType::BUFFER_TO_BUFFER, i, j});
                    }
                }
            }
        }
    }
    return moves;
}

// CUSTOM HEURISTIC

Move CustomHeuristic::calculate_move(BufferSimulator& sim) {
    World w = sim.getWorld();
    if (!w.arrival_stack.empty() && w.buffers[0].size() < w.max_buffer_size) {
        return Move{MoveType::ARRIVAL_TO_BUFFER, -1, 0};
    }
    return Move{MoveType::NONE};
}

// PRIORITY HEURISTIC

PriorityHeuristic::PriorityHeuristic(Tree::Tree* tree, std::vector<std::string>& terminal_names)
    : priority_tree(tree),
      terminals(terminal_names) {}

std::vector<double> PriorityHeuristic::extract_features(World& w) {

    std::vector<double> features;
    features.push_back(double(w.time));
    features.push_back(double(w.arrival_stack.size()));
    features.push_back(double(w.handover_stack.size()));

    for (int i = 0; i < 3; ++i) {
        features.push_back(double(w.buffers[i].size()));
    }

    // for (int i = 0; i < 3; ++i) {
    //     auto temp = w.buffers[i];
    //     while (!temp.empty()) {
    //         double ready = (double)temp.top().is_ready(w.time);
    //         features.push_back(ready);
    //         temp.pop();
    //     }
    // }
    return features;
}

double PriorityHeuristic::evaluate_move(BufferSimulator& sim, Move& m) {

    BufferSimulator sim_copy = sim;

    apply_move(sim_copy, m) ;

    World w = sim_copy.getWorld();
    auto features = extract_features(w);

    for (size_t i = 0; i < terminals.size(); ++i) {
        priority_tree->setTerminalValue(terminals[i], (void*)&features[i]);
    }

    double result;
    priority_tree->execute(&result);

    return result;
}


Move PriorityHeuristic::calculate_move(BufferSimulator& sim) {
    World w = sim.getWorld();
    std::vector<Move> moves = possible_moves(sim) ;

    Move best_move{MoveType::NONE, -1, -1};
    double best_score = -std::numeric_limits<double>::infinity();

    for (auto& m : moves) {
        double score = evaluate_move(sim, m);
        if (score > best_score) {
            best_score = score;
            best_move = m;
        }
    }
    return best_move;

}