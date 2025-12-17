#pragma once

#include "hotstorage_model.pb.h"
#include <vector>

namespace DynStacking {
    namespace HotStorage {

        using namespace DataModel;

        // Returns all legal crane moves for current world state
        std::vector<CraneMove> get_all_possible_moves(const World& world, bool optimized = true);

    }
}
