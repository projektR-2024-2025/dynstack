#include <iostream>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <ECF/ECF.h>
#include <cstdlib>

#include "hotstorage/hotstorage_model.pb.h"
#include "hotstorage/stacking.h"
#include "Model.h"

using std::cout;
using std::endl;

enum class Problem {
    Hotstorage
};

int main(int argc, char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Izvlacenje parametara
    const char* mode_env = std::getenv("MODE");
    std::string mode("HEURISTIC");
    if (mode_env)
        mode = mode_env;
    auto addr = argv[1];
    auto sim_id = argv[2];
    auto prob = argv[3];
    auto problem = Problem::Hotstorage;
    int MODEL = 2;
    XMLNode params = XMLNode::parseFile("/data/parameters.txt").getChildNode("ECF");
    if (!params.isEmpty()) {
        if (!params.getChildNodeByPath("Genotype/Tree").isEmpty())
            MODEL = 0;
        else if (!params.getChildNodeByPath("Genotype/Cartesian").isEmpty())
            MODEL = 1;
    }

    // Inicijalizacija ECFa
    ModelP model;
    if (mode.compare("GENETIC") == 0) {
        StateP state(new State);
        switch(MODEL) {
            // za GP
            case 0:
                model = std::make_shared<TreeModel>(); break;
            // za CGP
            case 1:
                model = std::make_shared<CGPModel>(); break;
            default:
                cout << "Unsupported MODEL!" << endl;
                return 2;
        }

        state->setEvalOp(model);
        char arg0[] = "state";
        char arg1[] = "/data/parameters.txt";
        char* ptrs[] = { arg0, arg1 };
        state->initialize(2, ptrs);

        XMLNode xInd = XMLNode::parseFile("/data/best.txt", "Individual");
        if (xInd.isEmpty()) {
            std::cout << "Exiting..." << std::endl;
            return 1;
        }
        IndividualP ind = (IndividualP) new Individual(state);
        ind->read(xInd);
        model->set_genotype(ind->getGenotype());
    }

    // Spajanje sa simulatorom
    zmq::context_t context;
    zmq::socket_t socket(context, zmq::socket_type::dealer);
    socket.set(zmq::sockopt::routing_id, sim_id);
    socket.connect(addr);
    cout << "connected to " << addr << " solving " << sim_id << endl;

    // Primanje stanja od simulatora i slanje odgovora
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
            if (mode.compare("HEURISTIC") == 0)
                answer = DynStacking::HotStorage::Heuristic::calculate_answer(msg[2].data(), msg[2].size());
            else if (mode.compare("GENETIC") == 0)
                answer = DynStacking::HotStorage::Genetic::calculate_answer(msg[2].data(), msg[2].size(), model);
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
