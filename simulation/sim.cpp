#include <iostream>
#include <stack>
#include <random>
#include <vector>

struct Container {
    int id, wait, overdue, arrival_time;
    Container(int id, int w, int o, int t) : id(id), wait(w), overdue(o), arrival_time(t) {}

	// funkcija koja nam provjerava je li kontejner spreman za premjestanje
    bool is_ready(int current_time) {
		return (current_time - arrival_time) >= wait;
    }

    // funkcija provjerava je li kontejner overdue
	// ako je vraca koliko je vremena overdue, inace vraca 0
    int is_overdue(int current_time) {
        int t = current_time - arrival_time;
		if (t > (overdue + wait)) {
			return t;
		}
        else {
            return 0;
        }
    }
};

class BufferSimulator {
private:
    int time = 0;
    std::stack<Container> arrival_stack, handover_stack;
    std::stack<Container> buffers[3];  // 3 buffer stacks
    std::mt19937 rng;
    int next_id = 1;
    const int max_buffer_size = 8;
    int processed_count = 0;

    void generate_arrival() {
        if (!arrival_stack.empty()) return; // u slucaju da je vec nesto na arrival_stacku
        std::uniform_int_distribution<> arrival_dist(0, 20);
        if (arrival_dist(rng) < 1) {  // Rare arrivals for manual control
            std::uniform_int_distribution<> wait_dist(1, 3);
            Container c(next_id++, wait_dist(rng), wait_dist(rng), time); // na temelju iste distribucije se generira i wait i overdue
            arrival_stack.push(c);
            std::cout << "Time " << time << ": Arrival #" << c.id << " (w=" << c.wait << ")" << std::endl;
        }
    }

    bool process_handover_top() {
        if (handover_stack.empty()) return false;
        std::uniform_int_distribution<> process_dist(0, 10);
        if (process_dist(rng) < 3) {
            Container c = handover_stack.top(); handover_stack.pop();
            processed_count++;
            std::cout << "Time " << time << ": PROCESSED #" << c.id
                << " (waited " << (time - c.arrival_time) << " steps)" << std::endl;
            return true;
        }
        return false;
    }

public:
    BufferSimulator() {
        std::random_device rd;
        rng.seed(rd());
    }

    // Manual move instructions
    bool move_arrival_to_buffer(int buffer_id) {
        if (arrival_stack.empty()) {
            std::cout << "Arrival stack empty!" << std::endl;
            return false;
        }
        if (buffer_id < 0 || buffer_id > 2 || buffers[buffer_id].size() >= max_buffer_size) {
            std::cout << "Invalid buffer or full!" << std::endl;
            return false;
        }
        
        Container c = arrival_stack.top(); arrival_stack.pop();
        buffers[buffer_id].push(c);
        std::cout << "Time " << time << ": #" << c.id << " -> Buffer " << buffer_id << std::endl;
        return true;
    }

    bool move_buffer_to_handover(int buffer_id) {
        if (buffer_id < 0 || buffer_id > 2 || buffers[buffer_id].empty()) {
            std::cout << "Invalid/empty buffer!" << std::endl;
            return false;
        }
        if (handover_stack.size() >= 3) {
            std::cout << "Handover full!" << std::endl;
            return false;
        }
        
        Container c = buffers[buffer_id].top(); buffers[buffer_id].pop();
        handover_stack.push(c);
        std::cout << "Time " << time << ": Buffer " << buffer_id << " #" << c.id << " -> Handover" << std::endl;
        return true;
    }

    void print_status() {
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
        std::cout << std::endl;
    }

    void step() {
        generate_arrival();
        process_handover_top();
        ++time;
    }
};

int main() {
    BufferSimulator sim;
    // demo proba
	for (int i = 0; i < 100; i++) {
		sim.step();
        sim.move_arrival_to_buffer(0);
		sim.print_status();
	}
    return 0;

    // dodati KPI
}
