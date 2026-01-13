#include <optional>
#include <ECF/ECF.h>

namespace DynStacking {
	namespace HotStorage {
		class Heuristic
		{
		public:
			static std::optional<std::string> calculate_answer(void* world_data, size_t len);
		};
		class Genetic
		{
		public:
			static std::optional<std::string> calculate_answer(void* world_data, size_t len, Tree::Tree* tree, std::vector<std::string>& terminal_names);
		};
	}
}