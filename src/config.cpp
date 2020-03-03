#include "config.hpp"

config::config() {
  opt_group{custom_options_, "global"}
    .add(output_file, "output-file,o", "Path for the output file")
    .add(include_hidden_actors, "include-hidden-actors,i",
         "Include hidden (system-level) actors")
    .add(verbosity, "verbosity,v", "Debug output (from 0 to 2)");
  // shutdown logging per default
  set("logger.verbosity", "quiet");
}
