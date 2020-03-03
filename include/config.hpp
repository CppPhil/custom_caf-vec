#include <string>

#include <caf/all.hpp>

struct config : public caf::actor_system_config {
  std::string output_file;
  bool include_hidden_actors = false;
  size_t verbosity = 0;
  config();
};
