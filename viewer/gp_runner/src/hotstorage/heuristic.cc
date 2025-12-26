#include "hotstorage_model.pb.h"
#include "heuristic.h"
#include <optional>
#include <vector>

namespace DynStacking {
    namespace HotStorage {
        using namespace DataModel;

        enum class MoveType {
            NONE,
            ARRIVAL_TO_BUFFER,
            BUFFER_TO_HANDOVER,
            BUFFER_TO_BUFFER
        };

        struct Move {
            MoveType type;
            int source;
            int target;
        };

        bool apply_move(World& w, const Move& m) {
            switch (m.type) {
            case MoveType::ARRIVAL_TO_BUFFER: {
                if (w.production().size() == 0) {
                    return false;
                }
                auto& block = w.production().bottomtotop(w.production().size() - 1);
                auto& buffer = *std::find_if(w.buffers().begin(), w.buffers().end(), [&](const Stack& s) { return s.id() == m.target; });
                if (buffer.bottomtotop().size() >= buffer.maxheight()) {
                    return false;
                }
                *buffer.add_bottomtotop() = block;
                w.mutable_production()->mutable_bottomtotop()->RemoveLast();
                return true;
            }
            case MoveType::BUFFER_TO_HANDOVER: {
                auto& buffer = *std::find_if(w.buffers().begin(), w.buffers().end(), [&](const Stack& s) { return s.id() == m.source; });
                if (buffer.bottomtotop().size() == 0) {
                    return false;
                }
                auto& block = buffer.bottomtotop(buffer.bottomtotop().size() - 1);
                if (!block.ready()) {
                    return false;
                }
                *w.mutable_handover()->add_ready() = block;
                buffer.mutable_bottomtotop()->RemoveLast();
                return true;
            }
            case MoveType::BUFFER_TO_BUFFER: {
                auto& src = *std::find_if(w.buffers().begin(), w.buffers().end(), [&](const Stack& s) { return s.id() == m.source; });
                auto& dst = *std::find_if(w.buffers().begin(), w.buffers().end(), [&](const Stack& s) { return s.id() == m.target; });
                if (src.bottomtotop().size() == 0) {
                    return false;
                }
                if (dst.bottomtotop().size() >= dst.maxheight()) {
                    return false;
                }
                auto& block = src.bottomtotop(src.bottomtotop().size() - 1);
                *dst.add_bottomtotop() = block;
                src.mutable_bottomtotop()->RemoveLast();
                return true;
            }
            case MoveType::NONE: default:
                return false;
            }
        }

        std::vector<Move> possible_moves(const World& w) {
            std::vector<Move> moves;
            if (w.production().size()) {
                for (auto& stack : w.buffers()) {
                    if (stack.bottomtotop().size() < stack.maxheight()) {
                        moves.push_back(Move{MoveType::ARRIVAL_TO_BUFFER, -1, stack.id()});
                    }
                }
            } else {
                // NONE
                moves.push_back(Move{MoveType::NONE, -1, -1});

                // BUFFER -> HANDOVER
                for (auto& stack : w.buffers()) {
                    if (stack.bottomtotop().size()) {
                        const Block& top = stack.bottomtotop(stack.bottomtotop().size() - 1);
                        if (top.ready()) {
                            moves.push_back(Move{MoveType::BUFFER_TO_HANDOVER, stack.id(), -1});
                        }
                    }
                }

                // BUFFER -> BUFFER
                for (auto& src : w.buffers()) {
                    if (src.bottomtotop().size()) {
                        for (auto& dst : w.buffers()) {
                            if (src.id() != dst.id()) {
                                if (dst.bottomtotop().size() < dst.maxheight()) {
                                    moves.push_back(Move{MoveType::BUFFER_TO_BUFFER, src.id(), dst.id()});
                                }
                            }
                        }
                    }
                }
            }
            return moves;
        }

        std::vector<double> extract_features(World& world){
            std::vector<double> features;

            features.push_back((double) world.now().milliseconds() * 0.001);
            features.push_back((double) world.production().bottomtotop().size());
            features.push_back((double) world.handover().ready());

            for (auto& stack : world.buffers())
                features.push_back((double) stack.bottomtotop().size());

            return features;
        }

        double evaluate_move(const World& world, Move& move, Tree::Tree* tree, std::vector<std::string>& terminal_names){
            World w_copy = world;
            apply_move(w_copy, move);
            auto features = extract_features(w_copy);

            for (size_t i = 0; i < terminal_names.size(); ++i) {
                tree->setTerminalValue(terminal_names[i], (void*)&features[i]);
            }

            double result;
            tree->execute(&result);

            return result;
        }

        Move calculate_move(World& world, Tree::Tree* tree, std::vector<std::string>& terminal_names) {
            auto moves = possible_moves(world);
            double best_value = -std::numeric_limits<double>::infinity();
            Move best_move = moves[0];

            for (auto& move : moves) {
                double value = evaluate_move(world, move, tree, terminal_names);
                if (value > best_value) {
                    best_value = value;
                    best_move = move;
                }
            }

            return best_move;
        }

        /// If any block on top of a stack can be moved to the handover schedule this move.
        // void any_handover_move(World& world, CraneSchedule& schedule) {
        //     if (!world.handover().ready()) {
        //         return;
        //     }
        //     for (auto& stack : world.buffers()) {
        //         int size = stack.bottomtotop().size();
        //         if (size == 0) {
        //             continue;
        //         }
        //         auto& top = stack.bottomtotop(size - 1);
        //         if (top.ready()) {
        //             auto move = schedule.add_moves();
        //             move->set_blockid(top.id());
        //             move->set_sourceid(stack.id());
        //             move->set_targetid(world.handover().id());
        //             return;
        //         }
        //     }
        // }

        // /// If the top block of the production stack can be put on a buffer schedule this move.
        // void clear_production_stack(World& world, CraneSchedule& schedule) {
        //     auto& src = world.production();
        //     int size = src.bottomtotop_size();
        //     if (size == 0) {
        //         return;
        //     }
        //     auto& top = src.bottomtotop(size - 1);
        //     auto tgt = std::find_if(world.buffers().begin(), world.buffers().end(), [](auto& s) { return s.maxheight() > s.bottomtotop_size(); });
        //     if (tgt != world.buffers().end()) {
        //         auto move = schedule.add_moves();
        //         move->set_blockid(top.id());
        //         move->set_sourceid(src.id());
        //         move->set_targetid(tgt->id());

        //     }
        // }

        std::optional<CraneSchedule> plan_moves(World& world, Tree::Tree* tree, std::vector<std::string>& terminal_names) {
            if (world.crane().schedule().moves_size() > 0) {
                // Leave the existing schedule alone
                return {};
            }
            CraneSchedule schedule;
            // any_handover_move(world, schedule);
            // clear_production_stack(world, schedule);

            auto best_move = calculate_move(world, tree, terminal_names);
            auto move = schedule.add_moves();
            auto& src = *std::find_if(world.buffers().begin(), world.buffers().end(), [&](const Stack& s) { return s.id() == best_move.source; });
            auto& dst = *std::find_if(world.buffers().begin(), world.buffers().end(), [&](const Stack& s) { return s.id() == best_move.target; });
            move->set_blockid(src.bottomtotop(src.bottomtotop_size() - 1).id());
            move->set_sourceid(src.id());
            move->set_targetid(dst.id());

            if (schedule.moves_size() > 0) {
                auto sequence = world.crane().schedule().sequencenr();
                schedule.set_sequencenr(sequence + 1);
                std::cout << schedule.DebugString() << std::endl;
                return schedule;
            }
            else {
                return {};
            }

        }

        std::optional<std::string> calculate_answer(void* world_data, size_t len, Tree::Tree* tree, std::vector<std::string>& terminal_names) {
            World world;
            world.ParseFromArray(world_data, len);
            auto plan = plan_moves(world, tree, terminal_names);
            if (plan.has_value()) {
                return plan.value().SerializeAsString();
            }
            else {
                return {};
            }
        }
    }
}
