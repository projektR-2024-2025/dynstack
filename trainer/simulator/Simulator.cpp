#include "Simulator.h"
#include "Heuristic.h"

void Simulator::initalize_buffers() {
    for (int i = 0; i < Parameters::BUFFER_COUNT; ++i) {
        std::uniform_int_distribution<> buff_dist(Parameters::MIN_INIT_BUFFER, Parameters::MAX_INIT_BUFFER);
        for (int j = 0; j < buff_dist(rng); j++) {
            std::uniform_int_distribution<> due_dist(Parameters::MIN_DUE_TIME, Parameters::MAX_DUE_TIME);
            int dueTime = due_dist(rng);
            std::uniform_real_distribution<> wait_dis(dueTime * Parameters::MIN_WAIT_FAC, dueTime * Parameters::MAX_WAIT_FAC);
            Container c(next_id++, static_cast<int>(wait_dis(rng)), dueTime, time);
            buffers[i].push(c);
        }
    }
}

void Simulator::generate_arrival() {
    std::uniform_int_distribution<> arrival_dist(0, 100);
    if (arrival_dist(rng) < Parameters::ARRIVAL_PROB) {
        if (arrival_stack.size() >= Parameters::MAX_ARRIVAL_SIZE) {  // u slucaju da je vec nesto na arrival_stacku
            KPI.blocked_arrival++;
            return;
        }
        std::uniform_int_distribution<> due_dist(Parameters::MIN_DUE_TIME, Parameters::MAX_DUE_TIME);
        int dueTime = due_dist(rng);
        std::uniform_real_distribution<> wait_dis(dueTime * Parameters::MIN_WAIT_FAC, dueTime * Parameters::MAX_WAIT_FAC);
        Container c(next_id++, static_cast<int>(wait_dis(rng)), dueTime, time);
        arrival_stack.push(c);
        if (Parameters::PRINT_STEPS)
            std::cout << "Time " << time << ": Arrival #" << c.id << " (w=" << c.wait << ")" << std::endl;
    }
}

bool Simulator::process_handover_top() {
    if (handover_stack.empty()) return false;
    std::uniform_int_distribution<> process_dist(0, 100);
    if (process_dist(rng) < Parameters::HANDOVER_PROB) {
        Container c = handover_stack.top(); handover_stack.pop();
        if (Parameters::PRINT_STEPS)
            std::cout << "Time " << time << ": PROCESSED #" << c.id << " (waited " << (time - c.arrival_time) << " steps)" << std::endl;
        return true;
    }
    return false;
}

void Simulator::calculate_KPI() {
    World world = getWorld();

    int total_blocks = 0;
    KPI.blocks_on_time = delivered_on_time;
    while (!world.arrival_stack.empty()) {
        if(!world.arrival_stack.front().is_overdue(time))
            KPI.blocks_on_time++;
        world.arrival_stack.pop();
        total_blocks++;
    }

    KPI.buffer_util = 0;
    for (int i = 0; i < Parameters::BUFFER_COUNT; i++) {
        KPI.buffer_util += static_cast<double>(world.buffers[i].size()) / Parameters::MAX_BUFFER_SIZE;
        while (!world.buffers[i].empty()) {
            if(!world.buffers[i].top().is_overdue(time))
                KPI.blocks_on_time++;
            world.buffers[i].pop();
            total_blocks++;
        }
    }
    KPI.buffer_util /= Parameters::BUFFER_COUNT;

    if (leadtime)
        KPI.leadtime -= static_cast<double>(KPI.leadtime - leadtime) / KPI.delivered_blocks;
    leadtime = 0;
    KPI.service_level = static_cast<double>(KPI.blocks_on_time) / (total_blocks + KPI.delivered_blocks);
    KPI.handover_util -= static_cast<double>(KPI.handover_util - !handover_stack.empty()) / time;
}

Simulator::Simulator() {
    last_move = MoveType::NONE;
    rng.seed(Simulator::seed);
    for (int i = 0; i < Parameters::BUFFER_COUNT; i++) {
        std::stack<Container> stck;
        buffers.push_back(stck);
    }
    if (Parameters::INITALIZE_BUFFERS)
        this->initalize_buffers();
}

World Simulator::getWorld() {
    return World{ time, arrival_stack, handover_stack, buffers, Parameters::MAX_ARRIVAL_SIZE, Parameters::MAX_BUFFER_SIZE, KPI };
}

// Manual move instructions
bool Simulator::move_arrival_to_buffer(int buffer_id) {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (arrival_stack.empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Arrival stack empty!" << std::endl;
        return false;
    }
    if (buffer_id < 0 || buffer_id > (Parameters::BUFFER_COUNT - 1) || static_cast<int>(buffers[buffer_id].size()) >= Parameters::MAX_BUFFER_SIZE) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid buffer or full!" << std::endl;
        return false;
    }

    KPI.crane_manipulations++;
    is_crane_avail = false;
    made_move = buffer_id + 1;
    Container c = arrival_stack.front(); arrival_stack.pop();
    buffers[buffer_id].push(c);
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": #" << c.id << " -> Buffer " << buffer_id << std::endl;

    last_move = MoveType::ARRIVAL_TO_BUFFER;

    return true;
}

bool Simulator::move_buffer_to_buffer(int from_buffer_id, int to_buffer_id) {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (from_buffer_id < 0 || from_buffer_id > (Parameters::BUFFER_COUNT - 1) || buffers[from_buffer_id].empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid/empty source buffer!" << std::endl;
        return false;
    }
    if (to_buffer_id < 0 || to_buffer_id > (Parameters::BUFFER_COUNT - 1) || static_cast<int>(buffers[to_buffer_id].size()) >= Parameters::MAX_BUFFER_SIZE) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid/full destination buffer!" << std::endl;
        return false;
    }

    KPI.crane_manipulations++;
    Container c = buffers[from_buffer_id].top();
    is_crane_avail = false;
    made_move = abs(from_buffer_id - to_buffer_id);
    buffers[from_buffer_id].pop();
    buffers[to_buffer_id].push(c);
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": Buffer " << from_buffer_id << " #" << c.id << " -> Buffer " << to_buffer_id << std::endl;

    last_move = MoveType::BUFFER_TO_BUFFER;

    return true;
}

bool Simulator::move_buffer_to_handover(int buffer_id) {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (buffer_id < 0 || buffer_id > (Parameters::BUFFER_COUNT - 1) || buffers[buffer_id].empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid/empty buffer!" << std::endl;
        return false;
    }
    if (!handover_stack.empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Handover full!" << std::endl;
        return false;
    }

    Container c = buffers[buffer_id].top();
    if (!c.is_ready(time)) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Container not ready!" << std::endl;
        return false;
    }

    KPI.crane_manipulations++;
    KPI.delivered_blocks++;
    leadtime = time - c.arrival_time;
    is_crane_avail = false;
    made_move = Parameters::BUFFER_COUNT - buffer_id;
    buffers[buffer_id].pop();
    handover_stack.push(c);
    if (!c.is_overdue(time))
        delivered_on_time++;
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": Buffer " << buffer_id << " #" << c.id << " -> Handover" << std::endl;

    last_move = MoveType::BUFFER_TO_HANDOVER;

    return true;
}

bool Simulator::move_arrival_to_handover() {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (arrival_stack.empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Arrival stack empty!" << std::endl;
        return false;
    }
    if (!handover_stack.empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Handover full!" << std::endl;
        return false;
    }

    Container c = arrival_stack.front();
    if (!c.is_ready(time)) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Container not ready!" << std::endl;
        return false;
    }

    KPI.crane_manipulations++;
    KPI.delivered_blocks++;
    leadtime = time - c.arrival_time;
    is_crane_avail = false;
    made_move = Parameters::BUFFER_COUNT + 1;
    arrival_stack.pop();
    handover_stack.push(c);
    if (!c.is_overdue(time))
        delivered_on_time++;
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": #" << c.id << " -> Handover" << std::endl;

    last_move = MoveType::ARRIVAL_TO_HANDOVER;

    return true;
}

bool Simulator::no_move() {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }

    last_move = MoveType::NONE;

    return false;
}

void Simulator::print_status() {
    std::cout << "\n--- Status at time " << time << " (Delivered: " << KPI.delivered_blocks << ") ---\n";
    std::cout << "Arrival: " << arrival_stack.size() << " | ";
    std::cout << "Buffers: [";
    for (int i = 0; i < Parameters::BUFFER_COUNT; ++i) {
        std::cout << buffers[i].size();
        if (i < (Parameters::BUFFER_COUNT - 1))
            std::cout << ",";
    }
    std::cout << "] | " << "Handover: " << handover_stack.size() << std::endl;

    // Show tops
    if (!arrival_stack.empty()) std::cout << "Arrival top: #" << arrival_stack.front().id << " ";
    for (int i = 0; i < Parameters::BUFFER_COUNT; ++i) {
        if (!buffers[i].empty()) std::cout << "B" << i + 1 << ": #" << buffers[i].top().id << " ";
    }
    if (!handover_stack.empty()) std::cout << "Handover: #" << handover_stack.top().id;
    std::cout << std::endl << "KPI values: ";
    KPI.print();
}

void Simulator::print_state() {
    World world = getWorld();
    std::cout << "\033[2J\033[H" << std::endl;
    for (int i = world.max_buffer_size; i >= 1; i--) {
        std::cout << std::left;

        if (i <= world.arrival_stack.size()) {
            if (world.arrival_stack.front().is_ready(world.time))
                std::cout << "\033[1;32m";
            if (world.arrival_stack.front().is_overdue(world.time))
                std::cout << "\033[1;31m";

            std::cout << std::setw(15) << world.arrival_stack.front().id << "\033[0m";
            world.arrival_stack.pop();
        }
        else
            std::cout << std::left << std::string(15, ' ');

        for (int j = 0; j < Parameters::BUFFER_COUNT; j++) {
            if (i <= world.buffers[j].size()) {
                if (world.buffers[j].top().is_ready(world.time))
                    std::cout << "\033[1;32m";
                if (world.buffers[j].top().is_overdue(world.time))
                    std::cout << "\033[1;31m";

                std::cout << std::setw(15) << world.buffers[j].top().id << "\033[0m";
                world.buffers[j].pop();
            }
            else
                std::cout << std::string(15, ' ');
        }

        if (i <= world.handover_stack.size())
            std::cout << std::setw(15) << world.handover_stack.top().id;
        else
            std::cout << std::string(15, ' ');

        std::cout << std::endl;
    }

    std::cout << std::left << std::setw(15) << "Arrival";
    for (int i = 0; i < Parameters::BUFFER_COUNT; ++i) {
        std::cout << "Buffer " << std::setw(8) << i + 1;
    }
    if (world.handover_stack.empty())
        std::cout << "\033[1;32m";
    std::cout << std::setw(15) << "Handover" << "\033[0m" << std::endl;
}

void Simulator::step() {
    generate_arrival();
    process_handover_top();
    ++time;
    is_crane_avail = (made_move > 0) ? false : true;
    made_move--;
    calculate_KPI();
}

void Simulator::seed_simulator() {
    std::random_device rd;
    Simulator::seed = rd();
}