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
                if (w.production().bottomtotop_size() == 0) {
                    return false;
                }
                const auto& block = w.production().bottomtotop(w.production().bottomtotop_size() - 1);
                auto buffer_it = std::find_if(w.mutable_buffers()->begin(), w.mutable_buffers()->end(),
                    [&](const Stack& s) { return s.id() == m.target; });
                if (buffer_it == w.mutable_buffers()->end()) {
                    return false;
                }
                auto& buffer = *buffer_it;
                if (buffer.bottomtotop_size() >= buffer.maxheight()) {
                    return false;
                }
                *buffer.add_bottomtotop() = block;
                w.mutable_production()->mutable_bottomtotop()->RemoveLast();
                return true;
            }
            case MoveType::BUFFER_TO_HANDOVER: {
                auto buffer_it = std::find_if(w.mutable_buffers()->begin(), w.mutable_buffers()->end(),
                    [&](const Stack& s) { return s.id() == m.source; });
                if (buffer_it == w.mutable_buffers()->end() || buffer_it->bottomtotop_size() == 0) {
                    return false;
                }
                const auto& block = buffer_it->bottomtotop(buffer_it->bottomtotop_size() - 1);
                if (!block.ready()) {
                    return false;
                }
                *w.mutable_handover()->mutable_block() = block;
                buffer_it->mutable_bottomtotop()->RemoveLast();
                return true;
            }
            case MoveType::BUFFER_TO_BUFFER: {
                auto src_it = std::find_if(w.mutable_buffers()->begin(), w.mutable_buffers()->end(),
                    [&](const Stack& s) { return s.id() == m.source; });
                auto dst_it = std::find_if(w.mutable_buffers()->begin(), w.mutable_buffers()->end(),
                    [&](const Stack& s) { return s.id() == m.target; });
                if (src_it == w.mutable_buffers()->end() || dst_it == w.mutable_buffers()->end() ||
                    src_it->bottomtotop_size() == 0 || dst_it->bottomtotop_size() >= dst_it->maxheight()) {
                    return false;
                }
                const auto& block = src_it->bottomtotop(src_it->bottomtotop_size() - 1);
                *dst_it->add_bottomtotop() = block;
                src_it->mutable_bottomtotop()->RemoveLast();
                return true;
            }
            case MoveType::NONE: default:
                return false;
            }
        }

        std::vector<Move> possible_moves(const World& w) {
            std::vector<Move> moves;
            if (w.production().bottomtotop_size()) {
                for (auto& stack : w.buffers()) {
                    if (stack.bottomtotop_size() < stack.maxheight()) {
                        moves.push_back(Move{MoveType::ARRIVAL_TO_BUFFER, -1, stack.id()});
                    }
                }
            } else {
                // BUFFER -> HANDOVER
                for (auto& stack : w.buffers()) {
                    if (stack.bottomtotop_size()) {
                        const Block& top = stack.bottomtotop(stack.bottomtotop_size() - 1);
                        if (top.ready()) {
                            moves.push_back(Move{MoveType::BUFFER_TO_HANDOVER, stack.id(), -1});
                        }
                    }
                }

                // BUFFER -> BUFFER
                for (auto& src : w.buffers()) {
                    if (src.bottomtotop_size()) {
                        for (auto& dst : w.buffers()) {
                            if (src.id() != dst.id()) {
                                if (dst.bottomtotop_size() < dst.maxheight()) {
                                    moves.push_back(Move{MoveType::BUFFER_TO_BUFFER, src.id(), dst.id()});
                                }
                            }
                        }
                    }
                }
            }
            return moves;
        }

        std::vector<double> extract_features(const World& world) {
            std::vector<double> features;
            features.push_back(static_cast<double>(world.now().milliseconds()) * 0.001);
            features.push_back(static_cast<double>(world.production().bottomtotop_size()));
            features.push_back(static_cast<double>(world.handover().ready()));
            
            for (const auto& stack : world.buffers()) {
                features.push_back(static_cast<double>(stack.bottomtotop_size()));
            }
            return features;
        }

        double evaluate_move(const World& world, const Move& move, Tree::Tree* tree, std::vector<std::string>& terminal_names){
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

        Move calculate_move(const World& world, Tree::Tree* tree, std::vector<std::string>& terminal_names) {
            auto moves = possible_moves(world);
            if (moves.empty()) return {MoveType::NONE, -1, -1};
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

        std::optional<CraneSchedule> plan_moves(World& world, Tree::Tree* tree, std::vector<std::string>& terminal_names) {
            if (world.crane().schedule().moves_size() > 0) {
                return {};
            }
            
            CraneSchedule schedule;
            auto best_move = calculate_move(world, tree, terminal_names);
            
            if (best_move.type != MoveType::NONE) {
                auto* move = schedule.add_moves();
                
                if (best_move.type == MoveType::ARRIVAL_TO_BUFFER) {
                    if (world.production().bottomtotop_size() > 0) {
                        move->set_blockid(world.production().bottomtotop(world.production().bottomtotop_size() - 1).id());
                        move->set_sourceid(world.production().id());
                    }
                } else {
                    auto src_it = std::find_if(world.buffers().begin(), world.buffers().end(),
                                            [&](const Stack& s) { return s.id() == best_move.source; });
                    if (src_it != world.buffers().end() && src_it->bottomtotop_size() > 0) {
                        move->set_blockid(src_it->bottomtotop(src_it->bottomtotop_size() - 1).id());
                        move->set_sourceid(src_it->id());
                    }
                }

                if (best_move.type == MoveType::BUFFER_TO_HANDOVER) {
                    if (world.handover().ready()) {
                        move->set_targetid(world.handover().id());
                    }
                }
                else {
                    auto dst_it = std::find_if(world.buffers().begin(), world.buffers().end(),
                                            [&](const Stack& s) { return s.id() == best_move.target; });
                    if (dst_it != world.buffers().end() && dst_it->bottomtotop_size() < dst_it->maxheight()) {
                        move->set_targetid(dst_it->id());
                    }
                }
                
                schedule.set_sequencenr(1);
                std::cout << schedule.DebugString() << std::endl;
                return schedule;
            }
            
            return {};
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
