#include "get_all_possible_moves.h"

namespace DynStacking {
    namespace HotStorage {

        static bool has_free_space(const Stack& s) {
            return s.bottomtotop_size() < s.maxheight();
        }

        static bool stack_has_ready(const Stack& s) {
            for (const auto& b : s.bottomtotop())
                if (b.ready())
                    return true;
            return false;
        }

        std::vector<CraneMove> get_all_possible_moves(const World& world, bool optimized) {

            std::vector<CraneMove> moves;

            const Stack& production = world.production();
            const auto& buffers = world.buffers();
            const Handover& handover = world.handover();

            //Production → Buffer
            if (production.bottomtotop_size() > 0) {

                const Block& top_prod =
                    production.bottomtotop(production.bottomtotop_size() - 1);

                if (optimized) {
                    // first buffer with free space
                    for (const auto& buf : buffers) {
                        if (has_free_space(buf)) {
                            CraneMove m;
                            m.set_blockid(top_prod.id());
                            m.set_sourceid(production.id());
                            m.set_targetid(buf.id());
                            moves.push_back(m);
                            break;
                        }
                    }
                } else {
                    for (const auto& buf : buffers) {
                        if (!has_free_space(buf)) continue;

                        CraneMove m;
                        m.set_blockid(top_prod.id());
                        m.set_sourceid(production.id());
                        m.set_targetid(buf.id());
                        moves.push_back(m);
                    }
                }
            }

            //Buffer → Handover
            if (handover.ready()) {
                for (const auto& src : buffers) {
                    int n = src.bottomtotop_size();
                    if (n == 0) continue;

                    const Block& top = src.bottomtotop(n - 1);
                    if (!top.ready()) continue;

                    CraneMove m;
                    m.set_blockid(top.id());
                    m.set_sourceid(src.id());
                    m.set_targetid(handover.id());
                    moves.push_back(m);
                }
            }

            // Buffer → Buffer
            for (const auto& src : buffers) {
                int n = src.bottomtotop_size();
                if (n == 0) continue;

                const Block& top = src.bottomtotop(n - 1);

                for (const auto& tgt : buffers) {
                    if (src.id() == tgt.id()) continue;
                    if (!has_free_space(tgt)) continue;

                    if (optimized) {
                        int tgt_n = tgt.bottomtotop_size();
                        if (tgt_n > 0 && tgt.bottomtotop(tgt_n - 1).ready())
                            continue; // avoid burying READY
                    }

                    CraneMove m;
                    m.set_blockid(top.id());
                    m.set_sourceid(src.id());
                    m.set_targetid(tgt.id());
                    moves.push_back(m);
                }
            }

            return moves;
        }

    }
}
