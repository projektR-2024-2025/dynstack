#include "Heuristic.h"

// PRIORITY HEURISTIC

PriorityHeuristic::PriorityHeuristic(Tree::Tree* tree, std::vector<std::string>& terminal_names)
    : priority_tree(tree),
    terminals(terminal_names) {
}

// TODO
std::vector<double> PriorityHeuristic::extract_features(const World& before, const World& after, Move& m) {

    std::vector<double> features;

    //GLOBALNE (razlika prije i poslije poteza -> mozemo staviti samo poslije umjesto razlike ako zakljucimo da je tako bolje)
    double delta_arrival = double(after.arrival_stack.size() - before.arrival_stack.size());

    int ready_before = 0;
    int ready_after = 0;
    int blocks_before = before.arrival_stack.size();
    int blocks_after = after.arrival_stack.size();
    int lateness_before = 0;
    int lateness_after = 0;
    double overdue_before = 0.0;
    double overdue_after = 0.0;

    for (int b = 0; b < 3; ++b) {
        blocks_before += before.buffers[b].size();
        blocks_after += after.buffers[b].size();
        auto tmp = before.buffers[b];
        while (!tmp.empty()) {
            int tud = tmp.top().arrival_time + tmp.top().wait + tmp.top().overdue - before.time;
            if (tmp.top().is_overdue(before.time)) overdue_before += 1.0;
            if (tud < 0) lateness_before += tud;
            if (tmp.top().is_ready(before.time)) ready_before++;
            tmp.pop();
        }
        tmp = after.buffers[b];
        while (!tmp.empty()) {
            int tud = tmp.top().arrival_time + tmp.top().wait + tmp.top().overdue - after.time;
            if (tmp.top().is_overdue(after.time)) overdue_after += 1.0;
            if (tud < 0) lateness_after += tud;
            if (tmp.top().is_ready(after.time)) ready_after++;
            tmp.pop();
        }
    }
    double delta_total_ready = ready_after - ready_before;
    double delta_total_blocks = blocks_after - blocks_before;
    double delta_total_lateness = lateness_after - lateness_before;
    double delta_overdue_blocks = overdue_after - overdue_before;

    double handover_ready_before = before.handover_stack.empty();
    double handover_ready_after = after.handover_stack.empty();

    //LOKALNE (VEZANE UZ POTEZ)
    double moved_ready = 0.0; //jeli blok koji smo pomakli ready
    double moved_tud = 0.0; //tud bloka kojeg smo pomakli

    if (m.type == MoveType::ARRIVAL_TO_BUFFER && !before.arrival_stack.empty()) {
        auto c = before.arrival_stack.top();
        moved_ready = c.is_ready(before.time);
        moved_tud = c.arrival_time + c.wait + c.overdue - before.time;
    }
    if ((m.type == MoveType::BUFFER_TO_BUFFER || m.type == MoveType::BUFFER_TO_HANDOVER) && !before.buffers[m.from].empty()) {
        auto c = before.buffers[m.from].top();
        moved_ready = c.is_ready(before.time);
        moved_tud = c.arrival_time + c.wait + c.overdue - before.time;
    }
    //src
    std::stack<Container> src_stack;
    if (m.from >= 0 && m.from <= 2) src_stack = before.buffers[m.from];
    else if (m.type == MoveType::ARRIVAL_TO_BUFFER) src_stack = before.arrival_stack;
    double src_size = double(src_stack.size());
    double src_ready = 0.0; //koliko ima ready blokova na src
    double src_overdue = 0.0; //koliko ima overdue blokova na src
    while (!src_stack.empty()) {
        if (src_stack.top().is_ready(before.time)) src_ready += 1.0;
        if (src_stack.top().is_overdue(before.time)) src_overdue += 1.0;
        src_stack.pop();
    }
    //dest
    std::stack<Container> dest_stack;
    double dest_size = 0.0;
    double dest_ready = 0.0;
    double dest_overdue = 0.0;

    if (m.to >= 0 && m.to <= 2) dest_stack = after.buffers[m.to];
    else if (m.type == MoveType::BUFFER_TO_HANDOVER) dest_stack = after.handover_stack;
    dest_size = double(dest_stack.size());
    if (!dest_stack.empty()) {
        while (!dest_stack.empty()) {
            if (dest_stack.top().is_ready(after.time)) dest_ready += 1.0;
            if (dest_stack.top().is_overdue(after.time)) dest_overdue += 1.0;
            dest_stack.pop();
        }
    }

    features.push_back(delta_arrival);
    features.push_back(delta_total_ready);
    features.push_back(delta_total_blocks);
    features.push_back(delta_total_lateness);
    features.push_back(delta_overdue_blocks);
    features.push_back(handover_ready_before);
    features.push_back(handover_ready_after);
    features.push_back(moved_ready);
    features.push_back(moved_tud);
    features.push_back(src_size);
    features.push_back(src_ready);
    features.push_back(src_overdue);
    features.push_back(dest_size);
    features.push_back(dest_ready);
    features.push_back(dest_overdue);

    return features;
}

double PriorityHeuristic::evaluate_move(Simulator& sim, Move& m) {

    Simulator sim_copy = sim;
    World before = sim_copy.getWorld();

    apply_move(sim_copy, m);

    World after = sim_copy.getWorld();
    auto features = extract_features(before, after, m);

    for (size_t i = 0; i < terminals.size(); ++i) {
        priority_tree->setTerminalValue(terminals[i], (void*)&features[i]);
    }

    double result;
    priority_tree->execute(&result);

    return result;
}

Move PriorityHeuristic::calculate_move(Simulator& sim) {
    World w = sim.getWorld();
    std::vector<Move> moves = PriorityHeuristic::priority_possible_moves(sim);

    Move best_move{ MoveType::NONE, -1, -1 };
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

std::vector<Move> PriorityHeuristic::priority_possible_moves(Simulator& sim, int MetaAlgParam) {
    if (MetaAlgParam == 1) {
        // Pozovi prvog kandidata za Metalagoritam.
        return PriorityHeuristic::meta_alg_1(sim);
    }
    else if (MetaAlgParam == 2) {
        // Pozovi drugog kandidata za Metalgoritam.
        return PriorityHeuristic::meta_alg_2(sim);
    }
    else if (MetaAlgParam == 3) {
        // Poziv treceg kandidata za Metalgoritam.
        return PriorityHeuristic::meta_alg_3(sim);
    }
    else {
        return AbstractHeuristic::possible_moves(sim);
    }
}

std::vector<Move> PriorityHeuristic::meta_alg_1(Simulator& sim) {
    std::vector<Move> moves;
    return moves;
}

std::vector<Move> PriorityHeuristic::meta_alg_2(Simulator& sim) {
    std::vector<Move> moves;
    return moves;
}

std::vector<Move> PriorityHeuristic::meta_alg_3(Simulator& sim) {
    std::vector<Move> moves;
    return moves;
}