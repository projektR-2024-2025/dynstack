#include <iostream>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <ECF/ECF.h>

#include "hotstorage/hotstorage_model.pb.h"
#include "hotstorage/heuristic.h"
#include "SimulatorEvalOp.h"

using std::cout;
using std::endl;

enum class Problem {
    Hotstorage
};

int main(int argc, char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    StateP state(new State);
    state->setEvalOp(new SimulatorEvalOp);
    const char* arr[] = {"state", "/data/parameters.txt"};
    const char** ptr = arr;
    state->initialize(2, ptr);
    XMLNode xInd = XMLNode::parseFile("/data/best.txt", "Individual");
	IndividualP ind = (IndividualP) new Individual(state);
	ind->read(xInd);
    Tree::Tree* tree = (Tree::Tree*)ind->getGenotype().get();
	std::vector<std::string> terminal_names_ = { "t1", "t2", "t3", "t4", "t5", "t6" };

    auto addr = argv[1];
    auto sim_id = argv[2];
    auto prob = argv[3];
    auto problem = Problem::Hotstorage;

    zmq::context_t context;
    zmq::socket_t socket(context, zmq::socket_type::dealer);
    socket.set(zmq::sockopt::routing_id, sim_id);
    socket.connect(addr);
    cout << "connected to " << addr << " solving " << sim_id << endl;

    std::vector<zmq::message_t> msg;
    while (true) {
        msg.clear();
        if (!zmq::recv_multipart(socket, std::back_inserter(msg))) {
            return -1;
        }
        cout << "update" << endl;
        std::optional<std::string> answer;
        switch (problem) {
        case Problem::Hotstorage:
            answer = DynStacking::HotStorage::calculate_answer(msg[2].data(), msg[2].size(), tree, terminal_names_);
            break;
        }
        if (answer) {
            cout << "send" << endl;
            std::array<zmq::const_buffer, 3> msg = {
                zmq::str_buffer(""),
                zmq::str_buffer("crane"),
                zmq::buffer(answer.value())
            };
            if (!zmq::send_multipart(socket, msg)) {
                return -1;
            }
        } else {
            std::array<zmq::const_buffer, 3> msg = {
                zmq::str_buffer(""),
                zmq::str_buffer("crane"),
                zmq::str_buffer("")
            };
            if (!zmq::send_multipart(socket, msg)) {
                return -1;
            }
        }
    }
    return 0;
}
