#include <iostream>
#include <stack>
#include <random>
#include <vector>

struct Container {
    int id, width, arrival_time;
    Container(int id, int w, int t) : id(id), width(w), arrival_time(t) {}
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

public:
    BufferSimulator() {
        std::random_device rd;
        rng.seed(rd());
    }

    void generate_arrival() {
        std::uniform_int_distribution<> arrival_dist(0, 20);
        if (arrival_dist(rng) < 1) {  // Rare arrivals for manual control
            std::uniform_int_distribution<> width_dist(1, 3);
            Container c(next_id++, width_dist(rng), time);
            arrival_stack.push(c);
            std::cout << "Time " << time << ": Arrival #" << c.id << " (w=" << c.width << ")" << std::endl;
        }
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

    bool process_handover_top() {
        if (handover_stack.empty()) {
            std::cout << "Handover empty!" << std::endl;
            return false;
        }
        Container c = handover_stack.top(); handover_stack.pop();
        processed_count++;
        std::cout << "Time " << time << ": PROCESSED #" << c.id 
                  << " (waited " << (time - c.arrival_time) << " steps)" << std::endl;
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
        ++time;
    }

    void demo_sequence() {
        std::cout << "Demo: Manual container movement sequence...\n";
        print_status();
        
        // Example instructions
        step(); step();  // Wait for arrivals
        move_arrival_to_buffer(0);
        move_arrival_to_buffer(1);

        print_status();

        step(); step();
        move_arrival_to_buffer(2);
        
        move_buffer_to_handover(0);
        move_buffer_to_handover(1);
        process_handover_top();
        
        print_status();
    }
};

int main() {
    BufferSimulator sim;
    sim.demo_sequence();
    
    return 0;

    // prepraviti arival i handover
    // handover top sam
    // dodati KPI
}
