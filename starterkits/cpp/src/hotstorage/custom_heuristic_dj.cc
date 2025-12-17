#include "hotstorage_model.pb.h"
#include <optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <climits>
#include <ECF/ECF.h>


namespace DynStacking {
    namespace HotStorage {

        using namespace DataModel;


        long long TUD(const Block& b, const World& w) {
            return b.due().milliseconds() - w.now().milliseconds();
        }

        bool has_free_space(const Stack& s) {
            return s.bottomtotop_size() < s.maxheight();
        }

        // buffer s najmanje READY blokova 
        const Stack* buffer_with_least_ready(const World& world) {
            const Stack* best = nullptr;
            int best_cnt = INT_MAX;

            for (const auto& s : world.buffers()) {
                if (!has_free_space(s)) continue;

                int cnt = 0;
                for (const auto& blk : s.bottomtotop())
                    if (blk.ready()) cnt++;

                if (cnt < best_cnt) {
                    best_cnt = cnt;
                    best = &s;
                }
            }
            return best;
        }

        // READY blok na vrhu s najmanjim TUD
        std::optional<std::pair<const Block*, int>> best_ready_on_top(const World& world) {
            const Block* best = nullptr;
            int best_stack = -1;
            long long best_tud = LLONG_MAX;

            for (const auto& s : world.buffers()) {
                int n = s.bottomtotop_size();
                if (n == 0) continue;

                const auto& top = s.bottomtotop(n - 1);
                if (!top.ready()) continue;

                long long tud = TUD(top, world);
                if (tud < best_tud) {
                    best_tud = tud;
                    best = &top;
                    best_stack = s.id();
                }
            }

            if (best)
                return std::make_pair(best, best_stack);
            return {};
        }


        struct CoveredReady {
            int stack_id;
            int blocking_index;
            const Block* ready_block;
        };

        // pronadi ready blok najbliži vrhu, ali ne na vrhu
        std::optional<CoveredReady> covered_ready_block(const World& world) {
            CoveredReady best{};
            int best_depth = INT_MAX;

            for (const auto& s : world.buffers()) {
                int n = s.bottomtotop_size();
                if (n == 0) continue;

                // ignoriraj stog ako je READY blok na vrhu
                const auto& top = s.bottomtotop(n - 1);
                if (top.ready()) continue;

                // traži ready blokove ispod vrha
                for (int i = n - 2; i >= 0; --i) {
                    if (s.bottomtotop(i).ready()) {
                        int depth = n - i - 1;
                        if (depth < best_depth) {
                            best_depth = depth;
                            best.stack_id = s.id();
                            best.blocking_index = i + 1;
                            best.ready_block = &s.bottomtotop(i);
                        }
                        break; // samo najbliži vrhu
                    }
                }
            }
            if (best_depth < INT_MAX)
                return best;
            return {};
        }


        //buffer bez READY bloka na vrhu, s najmanje blokova
        const Stack* buffer_without_ready_on_top(const World& world, int forbidden_stack_id) {
            const Stack* best = nullptr;
            int best_size = INT_MAX;

            for (const auto& s : world.buffers()) {
                if (s.id() == forbidden_stack_id) continue;
                if (!has_free_space(s)) continue;

                int n = s.bottomtotop_size();
                if (n > 0 && s.bottomtotop(n - 1).ready()) continue;

                if (n < best_size) {
                    best_size = n;
                    best = &s;
                }
            }
            return best;
        }

        //GLAVNA HEURISTIKA

        std::optional<CraneSchedule> plan_moves(World& world) {

            if (world.crane().schedule().moves_size() > 0)
                return {};

            CraneSchedule schedule;

            //AKO postoji blok na arrival I ima mjesta na buffer
            const auto& prod = world.production();
            if (prod.bottomtotop_size() > 0) {
                const Stack* tgt = buffer_with_least_ready(world);
                if (tgt) {
                    const auto& blk = prod.bottomtotop(
                        prod.bottomtotop_size() - 1
                    );

                    std::cout << "[H1] Arrival block "
                            << blk.id()
                            << " -> buffer " << tgt->id()
                            << " (least READY)\n";

                    auto* m = schedule.add_moves();
                    m->set_blockid(blk.id());
                    m->set_sourceid(prod.id());
                    m->set_targetid(tgt->id());

                    schedule.set_sequencenr(
                        world.crane().schedule().sequencenr() + 1
                    );
                    return schedule;
                }
            }
            // handover ready i ima ready blokova na vrhu
            if (world.handover().ready()) {
                auto best = best_ready_on_top(world);
                if (best.has_value()) {
                    std::cout << "[H2] Ready block "
                            << best->first->id()
                            << " from stack " << best->second
                            << " -> HANDOVER (TUD="
                            << TUD(*best->first, world) << ")\n";

                    auto* m = schedule.add_moves();
                    m->set_blockid(best->first->id());
                    m->set_sourceid(best->second);
                    m->set_targetid(world.handover().id());

                    schedule.set_sequencenr(
                        world.crane().schedule().sequencenr() + 1
                    );
                    return schedule;
                }
            }

            //ready blok zaklonjen
            auto covered = covered_ready_block(world);
            if (covered.has_value()) {
                const Stack* tgt = buffer_without_ready_on_top(
                    world, covered->stack_id
                );

                if (tgt) {
                    const Stack& src = *std::find_if(
                        world.buffers().begin(),
                        world.buffers().end(),
                        [&](const Stack& s) {
                            return s.id() == covered->stack_id;
                        }
                    );

                    const auto& blocker =
                        src.bottomtotop(covered->blocking_index);

                    std::cout << "[H3] Uncover READY block "
                            << covered->ready_block->id()
                            << " by moving blocker "
                            << blocker.id()
                            << " -> buffer " << tgt->id()
                            << "\n";

                    auto* m = schedule.add_moves();
                    m->set_blockid(blocker.id());
                    m->set_sourceid(src.id());
                    m->set_targetid(tgt->id());

                    schedule.set_sequencenr(
                        world.crane().schedule().sequencenr() + 1
                    );
                    return schedule;
                }
            }

            std::cout << "[H] No applicable move\n";
            return {};
        }

        // calculate answer isti kao i u template heuristiki
        std::optional<std::string>
        calculate_answer(void* world_data, size_t len) {
            World world;
            world.ParseFromArray(world_data, len);

            auto plan = plan_moves(world);
            if (plan.has_value())
                return plan->SerializeAsString();

            return {};
        }

    } 
}
