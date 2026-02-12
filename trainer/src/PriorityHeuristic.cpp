#include "Heuristic.h"
#include "Model.h"

// PRIORITY HEURISTIC

PriorityHeuristic::PriorityHeuristic(ModelP model) : model_(model) {}

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

    //stack rankings
    const double TUD_EPS = 1.0;
    double min_tud[5];
    double avg_tud[5];
    bool empty[5];
    for (int i = 0; i < 5; ++i) {
        min_tud[i] = std::numeric_limits<double>::infinity();
        avg_tud[i] = std::numeric_limits<double>::infinity();
        empty[i] = true;
    }
    for (int i = 0; i < 5; ++i) {
        std::stack<Container> s;
        if (i == 0) s = before.arrival_stack;
        else if (i >= 1 && i <= 3) s = before.buffers[i - 1];
        else s = before.handover_stack;
        if (s.empty()) continue;
        double sum = 0.0;
        int cnt = 0;
        while (!s.empty()) {
            const auto& c = s.top();
            double tud = c.arrival_time + c.wait + c.overdue - before.time;
            if (tud >=0){ //gledamo samo nenegativne (oni koji vec kasne su manje bitni)
                min_tud[i] = std::min(min_tud[i], tud);
                sum += tud;
                cnt++;
            }
            s.pop();
        }
        if (cnt!=0) 
            avg_tud[i] = sum / cnt;
        empty[i] = false;
    }
    int emptying_priority[5] = {0, 0, 0, 0, 0}; //svaki stack ima svoj prioritet praznjenja ovisno o tome koliki je najmanji tud
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (i == j) continue;
            if (!empty[i] && empty[j]) {
                emptying_priority[i]++;
                continue;
            }
            if (empty[i] && !empty[j])
                continue;
            double d = min_tud[j] - min_tud[i];
            if (std::abs(d) < TUD_EPS) { //ako su najmanji tud-ovi slicni na oba stacka gledaj prosjecan
                if (avg_tud[j] > avg_tud[i])
                    emptying_priority[i]++;
            } else if (min_tud[j] > min_tud[i]) {
                emptying_priority[i]++;
            }
        }
    }
    int highest_emptying_priority_idx = 0;
    for (int i = 1; i < 5; ++i)
        if (emptying_priority[i] > emptying_priority[highest_emptying_priority_idx])
            highest_emptying_priority_idx = i;
    double highest_emptying_priority_tud = double(min_tud[highest_emptying_priority_idx]);

    //LOKALNE (VEZANE UZ POTEZ)
    double moved_ready = 0.0; //jeli blok koji smo pomakli ready
    double moved_tud = 0.0; //tud bloka kojeg smo pomakli

    if ((m.type == MoveType::ARRIVAL_TO_BUFFER || m.type == MoveType::ARRIVAL_TO_HANDOVER)&& !before.arrival_stack.empty()) {
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
    src_stack = (m.type == MoveType::ARRIVAL_TO_BUFFER || m.type == MoveType::ARRIVAL_TO_HANDOVER) ? before.arrival_stack : before.buffers[m.from];

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
    dest_stack = (m.type == MoveType::BUFFER_TO_HANDOVER || m.type == MoveType::ARRIVAL_TO_HANDOVER) ? before.handover_stack : before.buffers[m.to];
    double dest_size = 0.0;
    double dest_ready = 0.0;
    double dest_overdue = 0.0;
    dest_size = double(dest_stack.size());
    if (!dest_stack.empty()) {
        while (!dest_stack.empty()) {
            if (dest_stack.top().is_ready(after.time)) dest_ready += 1.0;
            if (dest_stack.top().is_overdue(after.time)) dest_overdue += 1.0;
            dest_stack.pop();
        }
    }

    int src_idx  = (m.type == MoveType::ARRIVAL_TO_BUFFER || m.type == MoveType::ARRIVAL_TO_HANDOVER) ? 0 : m.from + 1 ;
    int dest_idx = (m.type == MoveType::BUFFER_TO_HANDOVER || m.type == MoveType::ARRIVAL_TO_HANDOVER) ? 4 : m.to + 1;
    double src_emptying_priority = double(emptying_priority[src_idx]) ;
    double dest_emptying_priority = double(emptying_priority[dest_idx]) ;

    // razlika u kpi-jevima
    const KPI_t& kb = before.KPI;
    const KPI_t& ka = after.KPI;
    double delta_blocked_arrival = ka.blocked_arrival - kb.blocked_arrival;
    double delta_blocks_on_time = ka.blocks_on_time - kb.blocks_on_time;
    double delta_crane_manip = ka.crane_manipulations - kb.crane_manipulations;
    double delta_delivered = ka.delivered_blocks - kb.delivered_blocks;
    double delta_leadtime = ka.leadtime - kb.leadtime;
    double delta_service_level = ka.service_level - kb.service_level;
    double delta_buffer_util = ka.buffer_util - kb.buffer_util;
    double delta_handover_util = ka.handover_util - kb.handover_util;

    features.push_back(delta_arrival);
    features.push_back(delta_total_ready);
    features.push_back(delta_total_blocks);
    features.push_back(delta_total_lateness);
    features.push_back(delta_overdue_blocks);
    features.push_back(handover_ready_before);
    features.push_back(handover_ready_after);
    features.push_back(double(highest_emptying_priority_idx));
    features.push_back(highest_emptying_priority_tud);

    features.push_back(moved_ready);
    features.push_back(moved_tud);
    features.push_back(src_size);
    features.push_back(src_ready);
    features.push_back(src_overdue);
    features.push_back(src_emptying_priority);
    features.push_back(dest_size);
    features.push_back(dest_ready);
    features.push_back(dest_overdue);
    features.push_back(dest_emptying_priority);

    features.push_back(delta_blocked_arrival);
    features.push_back(delta_blocks_on_time);
    features.push_back(delta_crane_manip);
    features.push_back(delta_delivered);
    features.push_back(delta_leadtime);
    features.push_back(delta_service_level);
    features.push_back(delta_buffer_util);
    features.push_back(delta_handover_util);

    return features;
}

std::vector<double> PriorityHeuristic::selection(std::vector<double> features){
    std::vector<double> selected;
    std::istringstream ss(Parameters::SELECTED_FEATURES);
    int idx;
    //std::cout << "SELECTED_FEATURES='" << Parameters::SELECTED_FEATURES << "'" << std::endl; // debug
    while (ss >> idx){
        //std::cout << idx <<std::endl ;
        if (idx >= 1 && idx <= (int)features.size())
            selected.push_back(features[idx - 1]);
    }
    return selected ;
}

double PriorityHeuristic::evaluate_move(Simulator& sim, Move& m) {

    Simulator sim_copy = sim;
    World before = sim_copy.getWorld();

    apply_move(sim_copy, m);

    World after = sim_copy.getWorld();
    
    auto features = selection(extract_features(before, after, m));
    
    //for (size_t i = 0; i < terminals.size(); ++i) {
        //priority_tree->setterminalvalue(terminals[i], (void*)&features[i]);
    //}

    double result;
    //priority_tree->execute(&result);
	model_->execute(result, features);

    return result;
}

Move PriorityHeuristic::calculate_move(Simulator& sim) {
    World w = sim.getWorld();
    std::vector<Move> moves = PriorityHeuristic::priority_possible_moves(sim, Parameters::META_ALG);

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

std::vector<Move> PriorityHeuristic::possible_moves(Simulator& sim) {
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
        // Poziv za default heuristiku.
        return PriorityHeuristic::possible_moves(sim);
    }
}

// Force arival if stack has ony one space left
std::vector<Move> PriorityHeuristic::meta_alg_1(Simulator& sim) {
    std::vector<Move> moves;
    World w = sim.getWorld();

    if (w.max_arrival_size - w.arrival_stack.size() <= 1) {
        for (int b = 0; b < 3; ++b) {
            if (w.buffers[b].size() < w.max_buffer_size) {
                moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
            }
        }
    }
    else {
        // SKIP move
        moves.push_back(Move{ MoveType::NONE, -1, -1 });

        // ARRIVAL -> BUFFER
        for (int b = 0; b < 3; ++b) {
            if (w.buffers[b].size() < w.max_buffer_size) {
                moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
            }
        }

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

// Force arival to buffer if there is more than one container on arrival stack.
std::vector<Move> PriorityHeuristic::meta_alg_2(Simulator& sim) {
    std::vector<Move> moves;
    World w = sim.getWorld();

    if (w.arrival_stack.size() > 1) {
        for (int b = 0; b < 3; ++b) {
            if (w.buffers[b].size() < w.max_buffer_size) {
                moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
            }
        }
    }
    else {
        // SKIP move
        moves.push_back(Move{ MoveType::NONE, -1, -1 });

        // ARRIVAL -> BUFFER
        if (w.arrival_stack.size() == 1) {
            for (int b = 0; b < 3; ++b) {
                if (w.buffers[b].size() < w.max_buffer_size) {
                    moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
                }
            }
        }

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

std::vector<Move> PriorityHeuristic::meta_alg_3(Simulator& sim) {
    std::vector<Move> moves;
    World w = sim.getWorld();

    if (w.arrival_stack.size() > 1) {
        for (int b = 0; b < 3; ++b) {
            if (w.buffers[b].size() < w.max_buffer_size) {
                moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
            }
        }
        
        Container& arrival_top = w.arrival_stack.top();
        if (w.handover_stack.empty() && arrival_top.is_ready(w.time)) {
            moves.push_back(Move{ MoveType::ARRIVAL_TO_HANDOVER, -1, -1 });
        }
    }
    else {

        // Get possible handovers
        if (!w.arrival_stack.empty()) {
            Container& top = w.arrival_stack.top();
            if (top.is_ready(w.time) && w.handover_stack.empty()) {
                moves.push_back(Move{ MoveType::ARRIVAL_TO_HANDOVER, -1, -1 });
            }
        }
        for (int b = 0; b < 3; ++b) {
            if (!w.buffers[b].empty()) {
                Container& top = w.buffers[b].top();
                if (top.is_ready(w.time) && w.handover_stack.empty()) {
                    moves.push_back(Move{ MoveType::BUFFER_TO_HANDOVER, b, -1 });
                }
            }
        }
        // If any ready container is on top and handover is ready -> return moves
        if (!moves.empty()) return moves;


        // Rest of the possible moves.
        // SKIP move
        moves.push_back(Move{ MoveType::NONE, -1, -1 });

        // ARRIVAL -> BUFFER
        if (w.arrival_stack.size() == 1) {
            for (int b = 0; b < 3; ++b) {
                if (w.buffers[b].size() < w.max_buffer_size) {
                    moves.push_back(Move{ MoveType::ARRIVAL_TO_BUFFER, -1, b });
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