#include "simulator.h"

Container::Container(int id, int w, int o, int t) : id(id), wait(w), overdue(o), arrival_time(t) {}

// funkcija koja nam provjerava je li kontejner spreman za premjestanje
bool Container::is_ready(int current_time) const {
    return (current_time - arrival_time) > wait;
}

bool Container::is_overdue(int current_time) const {
    return (current_time - arrival_time) > (overdue + wait);
}

int Container::until_ready(int current_time) const {
    int t = current_time - arrival_time;
    if (t < wait)
        return wait - t;
    else
        return 0;
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

void BufferSimulator::initalize_buffers() {
    for (int i = 0; i < 3; ++i) {
        std::uniform_int_distribution<> buff_dist(2, 4);
        for (int j = 0; j < buff_dist(rng); j++) {
            std::uniform_int_distribution<> wait_dist(1, 15);
            Container c(next_id++, wait_dist(rng), wait_dist(rng), time);
            buffers[i].push(c);
        }
    }
}

void BufferSimulator::generate_arrival() {
    std::uniform_int_distribution<> arrival_dist(0, 20);
    if (arrival_dist(rng) < arrival_density) {
        if (!arrival_stack.empty()) {  // u slucaju da je vec nesto na arrival_stacku
            KPI[0]++;
            return;
        }
        std::uniform_int_distribution<> wait_dist(1, 15);
        Container c(next_id++, wait_dist(rng), wait_dist(rng), time); // na temelju iste distribucije se generira i wait i overdue
        arrival_stack.push(c);
        std::cout << "Time " << time << ": Arrival #" << c.id << " (w=" << c.wait << ")" << std::endl;
    }
}

bool BufferSimulator::process_handover_top() {
    if (handover_stack.empty()) return false;
    std::uniform_int_distribution<> process_dist(0, 10);
    if (process_dist(rng) < arrival_density) {
        Container c = handover_stack.top(); handover_stack.pop();
        processed_count++;
        std::cout << "Time " << time << ": PROCESSED #" << c.id
            << " (waited " << (time - c.arrival_time) << " steps)" << std::endl;
        return true;
    }
    return false;
}

BufferSimulator::BufferSimulator(int arrival_density = 1, bool initalize_buffers = false) 
    : arrival_density(arrival_density) {
    std::random_device rd;
    rng.seed(rd());
    if (initalize_buffers)
        this->initalize_buffers();
}

World BufferSimulator::getWorld() {
    return World{time, arrival_stack, handover_stack, {buffers[0], buffers[1], buffers[2]}, max_buffer_size, {KPI[0], KPI[1], KPI[2]}};
}

// Manual move instructions
bool BufferSimulator::move_arrival_to_buffer(int buffer_id) {
    if (!isCraneAvail) {
        std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (arrival_stack.empty()) {
        std::cout << "Arrival stack empty!" << std::endl;
        return false;
    }
    if (buffer_id < 0 || buffer_id > 2 || static_cast<int>(buffers[buffer_id].size()) >= max_buffer_size) {
        std::cout << "Invalid buffer or full!" << std::endl;
        return false;
    }
    
    KPI[2]++;
    isCraneAvail = false;
    Container c = arrival_stack.top(); arrival_stack.pop();
    buffers[buffer_id].push(c);
    std::cout << "Time " << time << ": #" << c.id << " -> Buffer " << buffer_id << std::endl;
    return true;
}

bool BufferSimulator::move_buffer_to_buffer(int from_buffer_id, int to_buffer_id) {
    if (!isCraneAvail) {
        std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (from_buffer_id < 0 || from_buffer_id > 2 || buffers[from_buffer_id].empty()) {
        std::cout << "Invalid/empty source buffer!" << std::endl;
        return false;
    }
    if (to_buffer_id < 0 || to_buffer_id > 2 || static_cast<int>(buffers[to_buffer_id].size()) >= max_buffer_size) {
        std::cout << "Invalid/full destination buffer!" << std::endl;
        return false;
    }
    
    KPI[2]++;
    Container c = buffers[from_buffer_id].top();
    isCraneAvail = false;
    buffers[from_buffer_id].pop();
    buffers[to_buffer_id].push(c);
    std::cout << "Time " << time << ": Buffer " << from_buffer_id << " #" << c.id << " -> Buffer " << to_buffer_id << std::endl;
    return true;
}

bool BufferSimulator::move_buffer_to_handover(int buffer_id) {
    if (!isCraneAvail) {
        std::cout << "Crane is not available!" << std::endl;
        return false;
    }
    if (buffer_id < 0 || buffer_id > 2 || buffers[buffer_id].empty()) {
        std::cout << "Invalid/empty buffer!" << std::endl;
        return false;
    }
    if (!handover_stack.empty()) {
        std::cout << "Handover full!" << std::endl;
        return false;
    }
    
    Container c = buffers[buffer_id].top();
    if (!c.is_ready(time)) {
        std::cout << "Container not ready!" << std::endl;
        return false;
    }

    KPI[2]++;
    isCraneAvail = false;
    buffers[buffer_id].pop();
    handover_stack.push(c);
    if(!c.is_overdue(time))
        KPI[1]++;
    std::cout << "Time " << time << ": Buffer " << buffer_id << " #" << c.id << " -> Handover" << std::endl;
    return true;
}

void BufferSimulator::print_status() {
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

void BufferSimulator::step() {
    generate_arrival();
    process_handover_top();
    ++time;
    isCraneAvail = true;
}

double BufferSimulator::run_simulation(BufferSimulator& sim, AbstractHeuristic& heuristic, int max_steps) {
    for(int step = 0; step < max_steps; ++step) {
        Move m = heuristic.calculate_move(sim);
        AbstractHeuristic::apply_move(sim, m);
        sim.step();
    }
    World w = sim.getWorld();
    return w.KPI[0] + w.KPI[1] + w.KPI[2];
}