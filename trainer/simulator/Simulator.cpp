#include "Simulator.h"

Container::Container(int id, int w, int o, int t) : id(id), wait(w), overdue(o), arrival_time(t) {}

// funkcija koja nam provjerava je li kontejner spreman za premjestanje
bool Container::is_ready(int current_time) const {
    return (current_time - arrival_time) > wait;
}

bool Container::is_overdue(int current_time) const {
    return (current_time - arrival_time) > (overdue + wait);
}

// funkcija provjerava je li kontejner overdue
// ako je vraca koliko je vremena overdue, inace vraca 0
int Container::get_overdue(int current_time) const {
    int t = current_time - arrival_time;
    if (t > (overdue + wait)) {
        return t - (overdue + wait);
    }
    else {
        return 0;
    }
}

void Simulator::initalize_buffers() {
    for (int i = 0; i < 3; ++i) {
        std::uniform_int_distribution<> buff_dist(Parameters::MIN_INIT_BUFFER, Parameters::MAX_INIT_BUFFER);
        for (int j = 0; j < buff_dist(rng); j++) {
            std::uniform_int_distribution<> wait_dist(Parameters::MIN_WAIT_TIME, Parameters::MAX_WAIT_TIME);
            Container c(next_id++, wait_dist(rng), wait_dist(rng), time);
            buffers[i].push(c);
        }
    }
}

void Simulator::generate_arrival() {
    std::uniform_int_distribution<> arrival_dist(0, 100);
    if (arrival_dist(rng) < Parameters::ARRIVAL_PROB) {
        if (arrival_stack.size() >= Parameters::MAX_ARRIVAL_SIZE) {  // u slucaju da je vec nesto na arrival_stacku
            KPI[0]++;
            return;
        }
        std::uniform_int_distribution<> wait_dist(Parameters::MIN_WAIT_TIME, Parameters::MAX_WAIT_TIME);
        Container c(next_id++, wait_dist(rng), wait_dist(rng), time); // na temelju iste distribucije se generira i wait i overdue
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
        processed_count++;
        if (Parameters::PRINT_STEPS)
            std::cout << "Time " << time << ": PROCESSED #" << c.id << " (waited " << (time - c.arrival_time) << " steps)" << std::endl;
        return true;
    }
    return false;
}

Simulator::Simulator() {
    rng.seed(Simulator::seed);
    if (Parameters::INITALIZE_BUFFERS)
        this->initalize_buffers();
}

World Simulator::getWorld() {
    return World{ time, arrival_stack, handover_stack, {buffers[0], buffers[1], buffers[2]}, Parameters::MAX_ARRIVAL_SIZE, Parameters::MAX_BUFFER_SIZE, {KPI[0], KPI[1], KPI[2]} };
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
    if (buffer_id < 0 || buffer_id > 2 || static_cast<int>(buffers[buffer_id].size()) >= Parameters::MAX_BUFFER_SIZE) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid buffer or full!" << std::endl;
        return false;
    }

    KPI[2]++;
    is_crane_avail = false;
    Container c = arrival_stack.top(); arrival_stack.pop();
    buffers[buffer_id].push(c);
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": #" << c.id << " -> Buffer " << buffer_id << std::endl;
    return true;
}

bool Simulator::move_buffer_to_buffer(int from_buffer_id, int to_buffer_id) {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (from_buffer_id < 0 || from_buffer_id > 2 || buffers[from_buffer_id].empty()) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid/empty source buffer!" << std::endl;
        return false;
    }
    if (to_buffer_id < 0 || to_buffer_id > 2 || static_cast<int>(buffers[to_buffer_id].size()) >= Parameters::MAX_BUFFER_SIZE) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Invalid/full destination buffer!" << std::endl;
        return false;
    }

    KPI[2]++;
    Container c = buffers[from_buffer_id].top();
    is_crane_avail = false;
    buffers[from_buffer_id].pop();
    buffers[to_buffer_id].push(c);
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": Buffer " << from_buffer_id << " #" << c.id << " -> Buffer " << to_buffer_id << std::endl;
    return true;
}

bool Simulator::move_buffer_to_handover(int buffer_id) {
    if (!is_crane_avail) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (buffer_id < 0 || buffer_id > 2 || buffers[buffer_id].empty()) {
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

    KPI[2]++;
    is_crane_avail = false;
    buffers[buffer_id].pop();
    handover_stack.push(c);
    if (!c.is_overdue(time))
        KPI[1]++;
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": Buffer " << buffer_id << " #" << c.id << " -> Handover" << std::endl;
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

    Container c = arrival_stack.top();
    if (!c.is_ready(time)) {
        if (Parameters::PRINT_STEPS)
            std::cout << "Container not ready!" << std::endl;
        return false;
    }

    KPI[2]++;
    is_crane_avail = false;
    arrival_stack.pop();
    handover_stack.push(c);
    if (!c.is_overdue(time))
        KPI[1]++;
    if (Parameters::PRINT_STEPS)
        std::cout << "Time " << time << ": #" << c.id << " -> Handover" << std::endl;
    return true;
}

void Simulator::print_status() {
    std::cout << "\n--- Status at time " << time << " (Processed: " << processed_count << ") ---\n";
    std::cout << "Arrival: " << arrival_stack.size() << " | ";
    std::cout << "Buffers: [" << buffers[0].size() << "," << buffers[1].size() << "," << buffers[2].size() << "] | ";
    std::cout << "Handover: " << handover_stack.size() << std::endl;

    // Show tops
    if (!arrival_stack.empty()) std::cout << "Arrival top: #" << arrival_stack.top().id << " ";
    for (int i = 0; i < 3; ++i) {
        if (!buffers[i].empty()) std::cout << "B" << i << ": #" << buffers[i].top().id << " ";
    }
    if (!handover_stack.empty()) std::cout << "Handover: #" << handover_stack.top().id;
    std::cout << std::endl << "KPI values:";

    for (int i = 0; i < 3; ++i)
        std::cout << KPI[i] << " ";
    std::cout << std::endl;
}

void Simulator::print_state() {
    World world = getWorld();
    std::cout << "\033[2J\033[H" << std::endl;
    // std::system(CLEAR_TERM);
    for (int i = world.max_buffer_size; i >= 1; i--) {
        std::cout << std::left;

        if (i <= world.arrival_stack.size()) {
            if (world.arrival_stack.top().is_ready(world.time))
                std::cout << "\033[1;32m";
            if (world.arrival_stack.top().is_overdue(world.time))
                std::cout << "\033[1;31m";

            std::cout << std::setw(15) << world.arrival_stack.top().id << "\033[0m";
            world.arrival_stack.pop();
        }
        else
            std::cout << std::left << std::string(15, ' ');

        for (int j = 0; j < 3; j++) {
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

    std::cout << std::left << std::setw(15) << "Arrival" << std::setw(15) << "Buffer 1"
        << std::setw(15) << "Buffer 2" << std::setw(15) << "Buffer 3";
    if (world.handover_stack.empty())
        std::cout << "\033[1;32m";
    std::cout << std::setw(15) << "Handover" << "\033[0m" << std::endl;
}

void Simulator::step() {
    generate_arrival();
    process_handover_top();
    ++time;
    is_crane_avail = true;
}

void Simulator::seed_simulator() {
    std::random_device rd;
    Simulator::seed = rd();
}