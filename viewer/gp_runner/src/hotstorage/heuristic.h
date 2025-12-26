#include <optional>

namespace DynStacking {
	namespace HotStorage {
		std::optional<std::string> calculate_answer(void* world_data, size_t len, Tree::Tree* tree, std::vector<std::string>& terminal_names);
	}
}