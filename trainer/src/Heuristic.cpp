#include "Heuristic.h"

bool AbstractHeuristic::apply_move(Simulator& sim, Move& m) {
    switch(m.type) {
        case MoveType::ARRIVAL_TO_BUFFER: return sim.move_arrival_to_buffer(m.to);
        case MoveType::BUFFER_TO_BUFFER: return sim.move_buffer_to_buffer(m.from, m.to);
        case MoveType::BUFFER_TO_HANDOVER: return sim.move_buffer_to_handover(m.from);
        case MoveType::NONE: default: return false;
    }
}

std::vector<Move> AbstractHeuristic::possible_moves(Simulator& sim) {
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
long long CustomHeuristic::TUD(const Container& c, World& w) {
    return (c.arrival_time + c.wait + c.overdue) - w.time;
}
bool CustomHeuristic::has_free_space(World& w, int buffer_id) {
    return w.buffers[buffer_id].size() < w.max_buffer_size;
}
// buffer s najmanje READY blokova 
int CustomHeuristic::buffer_with_least_ready(World& w) {
    int best = -1;
    int best_cnt = -1;
    for (int b = 0; b < 3; ++b) {
        if (!has_free_space(w, b)) continue;
        int cnt = 0;
        auto tmp = w.buffers[b];
        while (!tmp.empty()) {
            if (tmp.top().is_ready(w.time))
                cnt++;
            tmp.pop();
        }
        if (cnt < best_cnt || best_cnt==-1) {
            best_cnt = cnt;
            best = b;
        }
    }
    return best;
}
// READY blok na vrhu s najmanjim TUD
int CustomHeuristic::best_ready_on_top(World& w) {
    int best = -1;
    long long best_tud =-1;
    for (int b = 0; b < 3; ++b) {
        if (w.buffers[b].empty()) continue;
        Container& top = w.buffers[b].top();
        if (!top.is_ready(w.time)) continue;
        long long tud = TUD(top, w);
        if (tud < best_tud || best == -1) {
            best_tud = tud;
            best = b;
        }
    }
    return best;
}
// pronadi ready blok najbliži vrhu, ali ne na vrhu
bool CustomHeuristic::covered_ready_block(World& w, CoveredReady& out) {
    bool found = false;
    int best_depth = 0;
    for (int b = 0; b < 3; ++b) {
        auto tmp = w.buffers[b];
        int depth = 0;
        if (tmp.empty()) continue;
        if (tmp.top().is_ready(w.time)) continue;
        while (!tmp.empty()) {
            if (tmp.top().is_ready(w.time)) {
                if (!found || depth < best_depth) {
                    best_depth = depth;
                    out.buffer_id = b;
                    out.blocking_depth = depth;
                    found = true;
                }
                break; // samo najbliži vrhu
            }
            tmp.pop();
            depth++;
        }
    }
    return found;
}

//buffer bez READY bloka na vrhu, s najmanje blokova
int CustomHeuristic::buffer_without_ready_on_top(World& w, int forbidden) {
    int best = -1;
    int best_size = -1;
    for (int b = 0; b < 3; ++b) {
        if (b == forbidden) continue;
        if (!has_free_space(w, b)) continue;
        if (!w.buffers[b].empty() &&
            w.buffers[b].top().is_ready(w.time)) {
            continue;
        }
        int size = w.buffers[b].size();
        if (size < best_size || best==-1) {
            best_size = size;
            best = b;
        }
    }
    return best;
}

Move CustomHeuristic::calculate_move(Simulator& sim) {
    World w = sim.getWorld();
    // AKO postoji blok na arrival I ima mjesta na buffer
    if (!w.arrival_stack.empty()) {
        int tgt = buffer_with_least_ready(w);
        if (tgt != -1){
            std::cout << "[H1] Arrival block -> buffer " << tgt << " (least READY)\n";
            return Move{MoveType::ARRIVAL_TO_BUFFER, -1, tgt};
        }
    }   

    // handover ready i ima ready blokova na vrhu
    if (w.handover_stack.empty()) {
        int src = best_ready_on_top(w);
        if (src != -1){
            Container& blk = w.buffers[src].top();
            std::cout << "[H2] Ready block #" << blk.id 
                      << " from buffer " << src << " -> HANDOVER (TUD=" << TUD(blk, w) << ")\n";
            return Move{MoveType::BUFFER_TO_HANDOVER, src, -1};
        }
    }

    // ready blok zaklonjen
    CoveredReady covered;
    if (covered_ready_block(w, covered)) {
        int tgt = buffer_without_ready_on_top(w, covered.buffer_id);
        if (tgt != -1){
            std::cout << "[H3] Move top blocker from buffer " << covered.buffer_id << " -> buffer " << tgt << "\n";
            return Move{MoveType::BUFFER_TO_BUFFER, covered.buffer_id, tgt};
        }
    }
    std:cout <<"--Not doing anything...\n";
    return Move{MoveType::NONE, -1, -1};
}

// PRIORITY HEURISTIC

PriorityHeuristic::PriorityHeuristic(Tree::Tree* tree, std::vector<std::string>& terminal_names)
    : priority_tree(tree),
      terminals(terminal_names) {}

// TODO
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

double PriorityHeuristic::evaluate_move(Simulator& sim, Move& m) {

    Simulator sim_copy = sim;

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


Move PriorityHeuristic::calculate_move(Simulator& sim) {
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